/******************************************************************************
 * FTP-Server
 * Authors: Evan Myers, Justin Slind, Alex Tai, James Yoo
 * Date: November 2013
 *
 * Description:
 *   The functions found in this file create, connect, or accept sockets or
 *   socket connections. Functions that send or receive data from a socket are
 *   also found in this file. The PASV and PORT commands, which create and/or
 *   connect a socket to establish a data connection are found in this file.
 *****************************************************************************/
#include <arpa/inet.h>
#include <errno.h>
#include <ifaddrs.h>
#include <inttypes.h>
#include <netdb.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include "config.h"
#include "net.h"
#include "reply.h"
#include "session.h"


/******************************************************************************
 * Socket connection/creation constants.
 *****************************************************************************/
#define BACKLOG 50      /* Maximum number of clients queued for accept(2).
			 * TODO: Tie this value to the maximum concurrent user
			 *       count when implemented or higher, do some
			 *       research when the time comes. */


/******************************************************************************
 * PORT command error checking constants.
 *****************************************************************************/
/* The number of byte value fields in the PORT command argument.
 * h1,h2,h3,h4,h5,h6 = 6 comma separated fields. */
#define PORT_BYTE_ARGS 6

// The maximum 8-bit value
#define MAX_8_BIT  255


/******************************************************************************
 * Local function prototypes.
 *****************************************************************************/
static int get_pasv_sock (const char *address, const char *port);
static int port_connect (char *hostname, char *service);
static int get_port_address (int csfd, char **addr, char **portStr, char *arg);

/******************************************************************************
 * get_control_sock - see net.h
 *****************************************************************************/
int get_control_sock (void)
{
  // The socket that will listen for control connections from the client.
  int csfd;

  // Variables used to collect the IPv4 address of the server.
  char *interfaceSetting = "INTERFACE_CONFIG"; // ftp.conf setting
  char *interfaceResult;                       // value of the ftp.conf setting
  char interfaceAddr[INET_ADDRSTRLEN];

  // Variables used to collect the default port.
  char *portSetting = "DEFAULT_PORT_CONFIG"; // ftp.conf setting
  char *portResult;

  // Read the config file for the default port.
  if ((portResult = get_config_value (portSetting, FTP_CONFIG_FILE)) == NULL) {
    return -1;
  }

  // Read the config file to find which interface to use to make the socket.
  if ((interfaceResult = get_config_value (interfaceSetting,
					    FTP_CONFIG_FILE)) == NULL) {
    return -1;
  }

  /* Get the IPv4 address for the interface that was set in the configuration
   * file.*/
  if (get_interface_address (interfaceResult, &interfaceAddr) == -1) {
    free (interfaceResult);
    free (portResult);
    return -1;
  }
  free (interfaceResult);

  /* Create a control connection socket that is ready to accept a connection
   * from the client. */
  if ((csfd = get_pasv_sock (interfaceAddr, portResult)) == -1) {
    return -1;
  }
  free (portResult);
  
  return csfd;
}


/******************************************************************************
 * accept_connection - see net.h
 *****************************************************************************/
int accept_connection (int listenSfd, int mode, session_info_t *si)
{
  fd_set rfds;       // select() read file descriptor set.
  int stdinFd;       // store the fileno of stdin.
  int acceptedSfd;   // The socket returned by accept().


  // Used to check if the thread should return, for ACCEPT_PASV
  struct timeval timeout;
  struct timeval *timeoutPtr;
  int nready;                    // Used to check for select() timeout.


  // Ensure the listening socket is not the result of a previous error.
  if (listenSfd == -1)
    return -1;


  if (mode == ACCEPT_CONTROL) {
    // Collect the file descriptor for stdin, to detect server commands.
    if ((stdinFd = fileno (stdin)) == -1) {
      fprintf (stderr, "%s: fileno: %s\n", __FUNCTION__, strerror (errno));
      return -1;
    }
    /* There is no timeout. select() will block indefinitely a client is
     * attempting to create a control connection, or a command is entered
     * on the server console. */
    timeoutPtr = NULL;
  } else {
    /* select() will block until there is a data connection to accept(), or
     * it is time to check if the caller who created this command thread
     * wishes this command thread to terminate. */
    timeoutPtr = &timeout;
  }


  /* The loop condition will be checked more than once (ie. not be exited by a
   * break statement) when errno returns with value EINTR, or select() is
   * passed by timeout. */
  while (1) {
    FD_ZERO (&rfds);
    FD_SET (listenSfd, &rfds);
    // Allow main() to pass select() when there is a server command.
    if (mode == ACCEPT_CONTROL)
      FD_SET (stdinFd, &rfds);

    /* Set this value inside the loop to make Linux systems operate the same as
     * "most others". See 'man (2) select' for more information about the
     * timeout value. */
    if (mode == ACCEPT_PASV) {
      timeout.tv_sec = COM_THREAD_ABORT_TIMEOUT_SEC;
      timeout.tv_usec = COM_THREAD_ABORT_TIMEOUT_USEC;
    }


    /* mode: ACCEPT_PASV
     *    When a control thread  has called this function, the thread may be
     * requested to terminate by session(). 
     *
     * session() will send this request by setting cmd_abort to true (found 
     * in the structure passed as the third argument to this function).
     *
     * session() will set cmd_abort to true when requested to terminate by
     * main().
     *
     *
     * mode: ACCEPT_CONTROL
     *    When main() uses this function to accept a control connection, the
     * timeout will not be set, and select() may also be passed by input to
     * stdin. */
    if ((nready = select (listenSfd + 1, &rfds, NULL, NULL, timeoutPtr)) == -1){
      if (errno == EINTR)
	continue;
      fprintf (stderr, "%s: select: %s\n", __FUNCTION__, strerror (errno));
      return -1;
    }

    /* Return to exit the thread when session() requests the thread to
     * terminate. This check must occur before the timeout check. */
    if ((mode == ACCEPT_PASV) && (si->cmdAbort == true)) {
      if (close (listenSfd) == -1)
	fprintf (stderr, "%s: abort: %s\n", __FUNCTION__, strerror (errno));
      return -1;
    }

    // Restart the loop on timeout.
    if (nready == 0)
      continue;

    /* Return from the function to read a server command when there is input
     * in the stdin buffer. */
    if (mode == ACCEPT_CONTROL) {
      if (FD_ISSET (stdinFd, &rfds)) {
	return STDIN_READY;
      }
    }
    
    // Attempt to accept a connection with the named socket.
    if (FD_ISSET (listenSfd, &rfds)) {
      if ((acceptedSfd = accept (listenSfd, NULL, NULL)) == -1) {
	if (errno == EINTR)
	  continue;
	fprintf (stderr, "%s: accept: %s\n", __FUNCTION__, strerror (errno));
	return -1;
      }
      break; // A connection has been established, break the while loop.
    }
  }

  /* In this server implementation, the socket created with the PASV command is
   * intended to accept only one data connection. After a connection has been
   * accepted, close the listening socket. */
  if (mode == ACCEPT_PASV) {
    if (close (listenSfd) == -1)
      fprintf (stderr, "%s: ending close: %s\n", __FUNCTION__, strerror(errno));
  }
  
  return acceptedSfd;  // Return the accepted socket file descriptor.
}


/******************************************************************************
 * cmd_pasv - see net.h
 *****************************************************************************/
int cmd_pasv (session_info_t *si)
{
  // The setting to be searched for in the config file.
  char *interfaceSetting = "INTERFACE_CONFIG";
  // The value for the setting that was searched for in the config file.
  char *interfaceResult;
  // The IPv4 address of the configuration file interface.
  char interfaceAddr[INET_ADDRSTRLEN];

  int csfd = si->csfd;

  // Ensure the client has logged in.
  if (!si->loggedin) {
    send_mesg_530 (csfd, REPLY_530_REQUEST);
    return -1;
  }

  /* The server "MUST" close the data connection port when:
   * "The port specification is changed by a command from the user".
   * Source: RFC 959 page 19 */
  if (si->dsfd > 0) {
    if (close (si->dsfd) == -1)
      fprintf (stderr, "%s: close: %s\n", __FUNCTION__, strerror (errno));
    si->dsfd = 0;
  }

  // Read the config file to find which interface to use to make the socket.
  if ((interfaceResult = get_config_value (interfaceSetting,
					    FTP_CONFIG_FILE)) == NULL) {
    return -1;
  }

  /* Get the IPv4 address for the interface specified in the configuration
   * file. Free the interface string, it is no longer required. */
  if (get_interface_address (interfaceResult, &interfaceAddr) == -1) {
    free (interfaceResult);
    return -1;
  }
  free (interfaceResult);


  // Create a socket that will listen for a data connection from a client.
  if ((si->dsfd = get_pasv_sock (interfaceAddr, NULL)) == -1) {
    return -1;
  }

  // Send the data connection address information to the control socket.
  if (send_mesg_227 (csfd, si->dsfd) == -1) {
    close (si->dsfd);
    si->dsfd = 0;
    return -1;
  }
 
  // Accept a connection from the client on the listening socket.
  if ((si->dsfd = accept_connection (si->dsfd, ACCEPT_PASV, si)) == -1) {
  si->dsfd = 0;
  return -1;
  }

  return si->dsfd;
}


/******************************************************************************
 * get_interface_address - see net.h
 *****************************************************************************/
int get_interface_address (const char *interface,
			   char (*address)[INET_ADDRSTRLEN])
{
  struct ifaddrs *result, *iter;   // getifaddrs()
  void *tempAddrPtr;               // Void source pointer for inet_ntop().

  // Get a linked list of interface addresses.
  if (getifaddrs (&result) == -1) {
    fprintf (stderr, "%s: getifaddrs: %s\n", __FUNCTION__, strerror (errno));
    return -1;
  }

  // Iterate through the results of getifaddrs().
  for (iter = result; iter != NULL; iter = iter->ifa_next) {
    // Only IPv4 addresses have been implemented for our server, ignore IPv6.
    if (iter->ifa_addr->sa_family != AF_INET)
      continue;

    // Move to next node if the interface name does not match.
    if (strcmp (iter->ifa_name, interface) != 0)
      continue;

    /* At this point we have found the correct interface. Set the address
     * passed in the second argument to this function to the address of this
     * interface. */
    tempAddrPtr = &((struct sockaddr_in*)iter->ifa_addr)->sin_addr;
    inet_ntop (AF_INET, tempAddrPtr, *address, INET_ADDRSTRLEN);
    
    /* The address has been retrieved, no more iterations are necessary. */
    break;
  }

  // Return error when the interface was not found for IPv4.
  if (iter == NULL) {
    fprintf (stderr, "%s: '%s' is not a valid IPv4 interface\n",
	     __FUNCTION__, interface);
    freeifaddrs (result);
    return -1;
  }

  // Clean up and return from the function.
  freeifaddrs (result);
  return 0;
}


/******************************************************************************
 * Create a listening TCP IPv4 socket. The returned socket file descriptor is
 * ready to accept() a connection.
 *
 * Arguments:
 *    address - The IPv4 address to create the socket on (eg. "127.0.1.1").
 *    port    - The port to create the socket with. NULL can be passed to let
 *              the kernel choose the port.
 *
 * Return values:
 *    > 0   The file descriptor for the newly created data connection socket.
 *     -1   Error while creating the socket.
 *****************************************************************************/
static int get_pasv_sock (const char *address, const char *port)
{
  struct addrinfo hints, *result;   // getaddrinfo()
  int gai;         // getaddrinfo error string.
  int sfd;         // The file descriptor of the data connection socket.
  int optval;

  // Set the gettaddrinfo() hints.
  bzero (&hints, sizeof(hints));
  hints.ai_family = AF_INET;        // IPv4, four byte address
  hints.ai_socktype = SOCK_STREAM;  // the data connection is stream

  /* Populate the linked list found in result. Use the host name entry on the
   * routing table for the node argument. "0" in the second argument will
   * allow  the operating system to choose an available port when bind() is
   * called. */
  if ((gai = getaddrinfo (address, port, &hints, &result)) == -1) {
    fprintf (stderr, "%s: getaddrinfo: %s\n", __FUNCTION__, gai_strerror (gai));
    return -1;
  }

  /* Iterate through all elements of the linked list returned by getaddrinfo().
   * If any command required to create the data connection socket fails, try
   * the next element returned by getaddrinfo(). */
  if ((sfd = socket (result->ai_family,
		     result->ai_socktype,
		     result->ai_protocol)) == -1) {
    fprintf (stderr, "%s: socket: %s\n", __FUNCTION__, strerror (errno));
    return -1;
  }

  optval = 1;
  // Set the socket option to reuse port while in the TIME_WAIT state.
  if (setsockopt (sfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) ==-1){
    fprintf (stderr, "%s: setsockopt: %s\n", __FUNCTION__, strerror (errno));
    return -1;
  }

  /* Bind the socket to a port. The port is chosen by the operating system
   * with the call to bind() because "0" was passed to the getaddrinfo()
   * service argument. */
  if (bind (sfd, result->ai_addr, result->ai_addrlen) == -1) {
    fprintf (stderr, "%s: bind: %s\n", __FUNCTION__, strerror (errno));
    close (sfd);
    return -1;
  }

  // Allow the socket to accept() connections.
  if (listen (sfd, BACKLOG) == -1) {
    fprintf (stderr, "%s: listen: %s\n", __FUNCTION__, strerror (errno));
    close (sfd);
    return -1;
  }

  freeaddrinfo (result);  // Free the getaddrinfo() result
  return sfd;             // Return the listening data connection socket
}


/******************************************************************************
 * cmd_port - see net.h
 *****************************************************************************/
int cmd_port (session_info_t *si, char *arg)
{
  int argLen;  // The length of the command string.
  int csfd = si->csfd;

  // The data connection address to connect to.
  char *hostname = (si->connInfo).hostname;
  char *port = (si->connInfo).port;

  // The port command must have an argument.
  if (arg == NULL) {
    send_mesg_501 (csfd);
    return -1;
  }

  // Ensure the client has logged in.
  if (!si->loggedin) {
    send_mesg_530 (csfd, REPLY_530_REQUEST);
    return -1;
  }

  /* The server "MUST" close the data connection port when: 
   * "The port specification is changed by a command from the user".
   * Source: RFC 959 page 19 */
  if (si->dsfd != 0) {
    if (close (si->dsfd) == -1)
      fprintf (stderr, "%s: close: %s\n", __FUNCTION__, strerror (errno));
    si->dsfd = 0;
  }

  /* Filter invalid PORT arguments by comparing the length of the argument. Too
   * many or too little number of characters in the string means that the
   * argument is invalid. */
  if ((argLen = strlen (arg)) < (MIN_CMDPORT_ARG_STRLEN - 1)) {
    fprintf (stderr, "%s: PORT argument too short\n", __FUNCTION__);
    send_mesg_501 (csfd);
    return -1;
  } else if (argLen > (MAX_CMDPORT_ARG_STRLEN - 1)) {
    fprintf (stderr, "%s: PORT argument too long\n", __FUNCTION__);
    send_mesg_501 (csfd);
    return -1;
  }

  // Convert the address h1,h2,h3,h4,p1,p2 into a hostname and port.
  if (get_port_address (csfd, &hostname, &port, arg) == -1) {
    return -1;
  }

  // TODO: This section will go around the 426 message in transfer commands
  //       INSTEAD of this current location.
  // Establish the data connection to the hostname and service.
  if ((si->dsfd = port_connect (hostname, port)) == -1) {
    return -1;
  }
  send_mesg_200 (csfd, REPLY_200_PORT);
  
  // TODO: si->dsfd may NOT be set by this function.
  return si->dsfd;
}


/******************************************************************************
 * Convert the argument received with the PORT command to a hostname and
 * service string that can be used as arguments to getaddrinfo(). 
 *
 * The port command is entered as: PORT h1,h2,h3,h4,p1,p2 where h1-h4 are the
 * decimal values of each byte in the hostname and p1-p2 are the high and low
 * order bytes of the 16bit integer port.
 *
 * Arguments:
 *      csfd - The socket file descriptor for the control connection.
 *   address - The address string, passed as a pointer to this function, will
 *             be set to the IPv4 dot notation address on function return.
 *      port - The service string, passed as a pointer to this function, will
 *             be set to the port integer value expressed as a string on
 *             function return.
 *       arg - The port command argument. "h1,h2,h3,h4,p1,p2\n"
 *
 * Return values:
 *   0    The hostname and service strings have been successfully set.
 *  -1    Error, hostname and service strings are not set.
 *****************************************************************************/
static int get_port_address (int csfd, char **addr, char **portStr, char *arg)
{
  /* Used to collect each byte of the hostname and port that was passed to the
   * function in the string argument 'arg'.
   *
   * The bytes that will be collected from the command string and stored in the
   * elements of this array have been stored in a value that is larger than 8
   * bits. This has been done so that any byte value that is too large (an
   * invalid IPv4 address or port) can be found. */
  uint16_t h[PORT_BYTE_ARGS];    // h1,h2,h3,h4,p1,p2 see the function header.
  int hindex;

  int argLen;     // Length of the command string (function argument 4).
  uint16_t port;  // Stores the combined p1 and p2 value.
  int i;          // Loop counter.

  int charCounter; /* Used to ensure only one comma character is present 
		    * between each byte field in the command string. */
  
  // Initialize counters and compare values.
  argLen = strlen (arg);
  charCounter = 0;
  hindex = 0;
  i = 0;

  /* Process all characters in the command string. This includes the command
   * portion "PORT " and the argument portion "h1,h2,h3,h4,p1,p2\n". Check for
   * errors in the argument string. */
  while (i < argLen) {
    /* Enter this block if the current character is not a digit. All characters
     * that are not digits (0-9) will be processed in this block, and the loop
     * will be restarted with the continue statement to process the next
     * character. */
    if ((arg[i] < 48) || (arg[i] > 57)) {
      // Only one non-digit character may appear in one continuous sequence.
      if (charCounter == 1) {
	fprintf (stderr, "%s: illegal character in argument\n", __FUNCTION__);
	send_mesg_501 (csfd);
	return -1;
      }

      // Check the expected character locations for the expected characters.
      if (hindex == 0) {
	// The argument string must begin with an integer.
	fprintf (stderr, "%s: illegal character in argument\n", __FUNCTION__);
	send_mesg_501 (csfd);
      } else if (hindex < 6) {
	// Only a comma may separate each byte field.
	if (arg[i] != ',') {
	  fprintf (stderr, "%s: illegal character in argument\n", __FUNCTION__);
	  send_mesg_501 (csfd);
	}
      } else {
	// The last character must be a null terminator.
	if (arg[i] != '\0') {
	  fprintf (stderr, "%s: illegal character in argument\n", __FUNCTION__);
	  send_mesg_501 (csfd);
	}
      }
      // Increment counts on every character read.
      charCounter++;
      i++;
      continue;
    }

    /* Reset the count of continuous non-digit characters. The next character
     * is part of an integer. */
    charCounter = 0;

    /* When an integer is found, store the integer. See acknowledgement ONE in
     * the file header for the meaning of "SCNu16". */
    if (sscanf (arg + i, "%"SCNu16, &h[hindex]) == -1) {
      if (errno == EINTR) {
	i--; // So that the same character will be passed to sscanf()
	continue;
      }
      fprintf (stderr, "%s: sscanf: %s\n", __FUNCTION__, strerror (errno));
      return -1;
    }

    // Determine if the integer is too large to be stored in 1-byte.
    if (h[hindex] > MAX_8_BIT) {
      fprintf (stderr, "%s: invalid PORT argument, %"PRIu16" is larger than"
	       " a byte\n", __FUNCTION__, h[hindex]);
      send_mesg_501 (csfd);
      return -1;
    }

    // Determine how many digits are present in the integer.
    if (h[hindex] > 99) {
      i += 3;
    } else if (h[hindex] > 9) {
      i += 2;
    } else {
      i++;
    }

    // Store the next integer on the next iteration.
    hindex++;
  }

  // Ensure that the correct number of integers were present in the string.
  if (hindex < (PORT_BYTE_ARGS - 1)) {
    fprintf (stderr, "%s: improper PORT argument\n", __FUNCTION__);
    send_mesg_501 (csfd);
    return -1;
  }

  // Store the hostname in an IPv4 dot notation string..
  sprintf (*addr, "%"PRIu16".%"PRIu16".%"PRIu16".%"PRIu16,
	   h[0], h[1], h[2], h[3]);

  /* Multiply the value of the high order port byte by 256, to shift this byte
   * into the correct position. Works for big endian and little endian. */
  h[4] = (h[4] * 256);
  // Combine the two port bytes to create one integer.
  port = (h[4] | h[5]);
  // Store the integer as a string.
  sprintf (*portStr, "%"PRIu16, port);

  // The hostname and port have been set, return from the function.
  return 0;
}
  

/******************************************************************************
 * Connect to the address and port specified in the arguments received with the
 * PORT command.
 *
 * Arguments:
 *   hostname - The IPv4 address to connect to, represented in a dot notation
 *              string. (eg. "127.0.1.1")
 *    service - The service (port) to connect to, represented as a string.
 *              (eg. "56035")
 *
 * Return values:
 *   >0    The socket file descriptor of the newly created data connection.
 *   -1    Error, the connection could not be made.
 *****************************************************************************/
int port_connect (char *hostname, char *service)
{
  struct addrinfo hints, *result;   // getaddrinfo()
  int sfd;         // The file descriptor of the data connection socket.
  int gai;         // getaddrinfo() error string.

  // Set the gettaddrinfo() hints.
  bzero (&hints, sizeof(hints));
  hints.ai_family = AF_INET;        // IPv4, a four byte address.
  hints.ai_socktype = SOCK_STREAM;  // The data connection is stream.

  // Create the address information using the hostname and service.
  if ((gai = getaddrinfo (hostname, service, &hints, &result)) == -1) {
    fprintf (stderr, "%s: getaddrinfo: %s\n", __FUNCTION__, gai_strerror (gai));
    return -1;
  }

  // Create the socket with the values returned by getaddrinfo().
  if ((sfd = socket (result->ai_family,
		     result->ai_socktype,
		     result->ai_protocol)) == -1) {
    fprintf (stderr, "%s: socket: %s\n", __FUNCTION__, strerror (errno));
    freeaddrinfo (result);
    return -1;
  }

  // Connect to the client.
  if (connect (sfd, result->ai_addr, result->ai_addrlen) == -1) {
    fprintf (stderr, "%s: connect: %s\n", __FUNCTION__, strerror (errno));
    freeaddrinfo (result);
    return -1;
  }

  freeaddrinfo (result);
  return sfd;
}


/******************************************************************************
 * send_all - see net.h
 *****************************************************************************/
int send_all (int sfd, uint8_t *mesg, int toSend)
{
  int nsent = 0; 

  while (toSend > 0) {
    if ((nsent = send (sfd, mesg, toSend, 0)) == -1) {
      if (errno == EINTR)
	continue;
      fprintf (stderr, "%s: %s\n", __FUNCTION__, strerror (errno));
      return -1;
    }
    // Update the number of bytes to send to the socket.
    toSend -= nsent;
    nsent = 0;
  }

  return 0;
}
