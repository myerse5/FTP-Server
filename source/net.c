/******************************************************************************
 * Students: Evan Myers, Justin Slind, Alex Tai, James Yoo
 * Course: CMPT-361
 * Assignment #3 - ftp server
 * File: net.c
 * Date: November 2013
 *
 * Description:
 *   The functions found in this file create, connect, or accept sockets or
 *   socket connections. Functions that send or recieve data from a socket are
 *   also found in this file. The PASV and PORT commands, which create and/or
 *   connect a socket to establish a data connection are found in this file.
 *
 * Acknowledgements:
 *   ONE - I have discovered that the printf/scanf family has a conversion
 *         specifier for type uinN_t: http://stackoverflow.com/a/6993177
 *         I have used the ideas in this example in my code. Search for "ONE"
 *         in this file for the code blocks that use the macro.
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
#define BACKLOG 10      //Maximum number of clients queued for accept(2).


/******************************************************************************
 * PORT command error checking constants.
 *****************************************************************************/
/* The maximum string length of a PORT command (including the arguments).
 * (6 three digit fields) + (5 commas) + (null terminator) */
#define MAX_PORT_STR_LEN ((6*3) + 5 + 1)

/* The minimum string length of a PORT command (includeing the arguments).
 * (PORT + space) + (6 one digit fields) + (5 commas) + (newline + null) */
#define MIN_PORT_STR_LEN ((6*1) + 5 + 1)

//The number of byte values in the PORT command argument. PORT h1,h2,h3,h4,h5,h6
#define PORT_BYTE_ARGS 6


/******************************************************************************
 * Various maximum value constants.
 *****************************************************************************/
#define MAX_8_BIT  255  //The maximum 8-bit value


/******************************************************************************
 * Local function prototypes.
 *****************************************************************************/
//used by cmd_pasv()
static int get_pasv_sock (const char *address, const char *port);

//used by cmd_port()
static int get_port_address (int c_sfd,
			     char (*hostname)[INET_ADDRSTRLEN], 
			     char (*service)[MAX_PORT_STR], 
			     char *cmd_str);

//used by cmd_port()
static int port_connect (char *hostname, char *service);


/******************************************************************************
 * get_control_sock
 *****************************************************************************/
int get_control_sock (void)
{
  //The socket that will listen for control connections from the client.
  int c_sfd;

  //Variables used to collect the IPv4 address of the server.
  char *interface_setting = "INTERFACE_CONFIG"; //ftp.conf setting
  char *interface_result;                       //value of the ftp.conf setting
  char interface_addr[INET_ADDRSTRLEN];

  //Variables used to collect the default port.
  char *port_setting = "DEFAULT_PORT_CONFIG"; //ftp.conf setting
  char *port_result;

  //Read the config file for the default port.
  if ((port_result = get_config_value (port_setting, 
				       FTP_CONFIG_FILE)) == NULL) {
    return -1;
  }

  //Read the config file to find which interface to use to make the socket.
  if ((interface_result = get_config_value (interface_setting,
					    FTP_CONFIG_FILE)) == NULL) {
    return -1;
  }

  /* Get the IPv4 address for the interface that was set in the configuration
   * file.*/
  if (get_interface_address (interface_result, &interface_addr) == -1) {
    free (interface_result);
    free (port_result);
    return -1;
  }
  free (interface_result);

  /* Create a data connection socket that is ready to accept a connection from
   * the client. */
  if ((c_sfd = get_pasv_sock (interface_addr, port_result)) == -1) {
    return -1;
  }
  free (port_result);
  
  return c_sfd;
}


/******************************************************************************
 * accept_connection - see net.h
 *****************************************************************************/
int accept_connection (int listen_sfd, int mode, session_info_t *si)
{
  fd_set rfds;       //select() read file descriptor set.
  int stdin_fd;      //store the fileno of stdin.
  int accepted_sfd;  //the socket returned by accept()


  //Used to check if the thread should return, for ACCEPT_PASV
  struct timeval timeout;
  struct timeval *timeout_ptr;
  int nready;                    //Used to check for select() timeout.


  //Ensure the listening socket is not the result of a previous error.
  if (listen_sfd == -1)
    return -1;


  if (mode == ACCEPT_CONTROL) {
    //Collect the file descriptor for stdin, to detect server commands.
    if ((stdin_fd = fileno (stdin)) == -1) {
      fprintf (stderr, "%s: fileno: %s\n", __FUNCTION__, strerror (errno));
      return -1;
    }


    /* There is no timeout. select() will block indefinately a client is
     * attempting to create a control connection, or a command is entered
     * on the server console. */
    timeout_ptr = NULL;
  } else {
    /* select() will block until there is a data connection to accept(), or
     * it is time to check if the caller who created this command thread
     * wishes this command thread to terminate. */
    timeout_ptr = &timeout;
  }


  /* The loop condition will be checked more than once (ie. not be exited by a
   * break statement) when errno returns with value EINTR, or select() is
   * passed by timeout. */
  while (1) {
    FD_ZERO (&rfds);
    FD_SET (listen_sfd, &rfds);
    //allow main() to pass select() when there is a server command.
    if (mode == ACCEPT_CONTROL)
      FD_SET (stdin_fd, &rfds);

    /* Set this value inside the loop to make linux systems operate the same as
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
    if ((nready = select (listen_sfd + 1, &rfds, 
			  NULL, NULL, timeout_ptr)) == -1) {
      if (errno == EINTR)
	continue;
      fprintf (stderr, "%s: select: %s\n", __FUNCTION__, strerror (errno));
      return -1;
    }

    /* Return to exit the thread when session() requests the thread to
     * terminate. This check must occur before the timeout check. */
    if ((mode == ACCEPT_PASV) && (si->cmd_abort == true)) {
      if (close (listen_sfd) == -1)
	fprintf (stderr, "%s: abort close: %s\n", __FUNCTION__,strerror(errno));
      return -1;
    }

    //Restart the loop on timeout.
    if (nready == 0)
      continue;

    /* Return from the function to read a server command when there is input
     * in the stdin buffer. */
    if (mode == ACCEPT_CONTROL) {
      if (FD_ISSET (stdin_fd, &rfds)) {
	return STDIN_READY;
      }
    }
    
    //Attempt to accept a connection with the named socket.
    if (FD_ISSET (listen_sfd, &rfds)) {
      if ((accepted_sfd = accept (listen_sfd, NULL, NULL)) == -1) {
	if (errno == EINTR)
	  continue;

	fprintf (stderr, "%s: accept: %s\n", __FUNCTION__, strerror (errno));
	return -1;
      }
      break; //while loop
    }
  }

  /* In this server implementation, the socket created with the PASV command is
   * intended to accept only one data connection. After a connection has been
   * accepted, close the listening socket. */
  if (mode == ACCEPT_PASV) {
    if (close (listen_sfd) == -1)
      fprintf (stderr, "%s: ending close: %s\n", __FUNCTION__, strerror(errno));
  }
  
  return accepted_sfd;  //Return the accepted socket file descriptor.
}


/******************************************************************************
 * cmd_pasv - see net.h
 *****************************************************************************/
int cmd_pasv (session_info_t *session)
{
  //The setting to be searched for in the config file.
  char *interface_setting = "INTERFACE_CONFIG";
  //The value for the setting that was searched for in the config file.
  char *interface_result;
  //The IPv4 address of the configuration file interface.
  char interface_addr[INET_ADDRSTRLEN];

  //Ensure the client has logged in.
  if (!session->logged_in) {
    send_mesg_530 (session->c_sfd);
    return -1;
  }

  /* The server "MUST" close the data connection port when:
   * "The port specification is changed by a command from the user".
   * Source: rfc 959 page 19 */
  if (session->d_sfd > 0) {
    if (close (session->d_sfd) == -1)
      fprintf (stderr, "%s: close: %s\n", __FUNCTION__, strerror (errno));
    session->d_sfd = 0;
  }

  //Read the config file to find which interface to use to make the socket.
  if ((interface_result = get_config_value (interface_setting,
					    FTP_CONFIG_FILE)) == NULL) {
    return -1;
  }

  /* Get the IPv4 address for the interface specified in the configuration
   * file. Free the interface string, it is no longer required. */
  if (get_interface_address (interface_result, &interface_addr) == -1) {
    free (interface_result);
    return -1;
  }
  free (interface_result);


  //Create a socket that will listen for a data connection from a client.
  if ((session->d_sfd = get_pasv_sock (interface_addr, NULL)) == -1) {
    return -1;
  }

  //Send the data connection address information to the control socket.
  if (send_mesg_227 (session->c_sfd, session->d_sfd) == -1) {
    close (session->d_sfd);
    session->d_sfd = 0;
    return -1;
  }
 
  //Accept a connection from the client on the listening socket.
  if ((session->d_sfd = accept_connection (session->d_sfd,
					   ACCEPT_PASV,
					   session)) == -1) {
    session->d_sfd = 0;
    return -1;
  }

  return session->d_sfd;
}


/******************************************************************************
 * get_interface_address - see net.h
 *****************************************************************************/
int get_interface_address (const char *interface,
			   char (*address)[INET_ADDRSTRLEN])
{
  struct ifaddrs *result, *iter;   //getifaddrs()
  void *tmp_addr_ptr;              //Void source pointer for inet_ntop().

  //Get a linked list of interface addresses.
  if (getifaddrs (&result) == -1) {
    fprintf (stderr, "%s: getifaddrs: %s\n", __FUNCTION__, strerror (errno));
    return -1;
  }

  //Iterate through the results of getifaddrs().
  for (iter = result; iter != NULL; iter = iter->ifa_next) {
    //Only IPv4 addresses have been implemented for our server, ignore IPv6.
    if (iter->ifa_addr->sa_family != AF_INET)
      continue;

    //Move to next node if the interface name does not match.
    if (strcmp (iter->ifa_name, interface) != 0)
      continue;

    /* At this point we have found the correct interface. Set the address
     * passed in the second argument to this function to the address of this
     * interface. */
    tmp_addr_ptr = &((struct sockaddr_in *)iter->ifa_addr)->sin_addr;
    inet_ntop(AF_INET, tmp_addr_ptr, *address, INET_ADDRSTRLEN);
    
    /* The address has been retrieved, no more iterations are necessary. */
    break;
  }

  //Return error when the interface was not found for IPv4.
  if (iter == NULL) {
    fprintf (stderr, "%s: '%s' is not a valid IPv4 interface\n",
	     __FUNCTION__, interface);
    freeifaddrs (result);
    return -1;
  }

  //Clean up and return from the function.
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
 *
 * Original author: Evan Myers
 *****************************************************************************/
static int get_pasv_sock (const char *address, const char *port)
{
  struct addrinfo hints, *result;   //getaddrinfo()
  int gai;         //getaddrinfo error string.
  int sfd;         //the file descriptor of the data connection socket
  int optval;

  //set the gettaddrinfo() hints
  bzero (&hints, sizeof(hints));
  hints.ai_family = AF_INET;        //IPv4, four byte address
  hints.ai_socktype = SOCK_STREAM;  //the data connection is stream

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
  //Set the socket option to reuse port while in the TIME_WAIT state.
  if (setsockopt (sfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof (optval))
      == -1) {
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

  //Allow the socket to accept() connections.
  if (listen (sfd, BACKLOG) == -1) {
    fprintf (stderr, "%s: listen: %s\n", __FUNCTION__, strerror (errno));
    close (sfd);
    return -1;
  }

  freeaddrinfo (result);  //Free the getaddrinfo() result
  return sfd;             //Return the listening data connection socket
}


/******************************************************************************
 * cmd_port - see net.h
 *****************************************************************************/
int cmd_port (session_info_t *session, char *cmd_str)
{
  int cmd_str_len;  //The length of the command string.
  //The data connection address to connect to.
  char hostname[INET_ADDRSTRLEN];  //Maximum size of an IPv4 dot notation addr.
  char service[MAX_PORT_STR];

  //The port command must have an argument.
  if (cmd_str == NULL) {
    send_mesg_501 (session->c_sfd);
    return -1;
  }

  //Ensure the client has logged in.
  if (!session->logged_in) {
    send_mesg_530 (session->c_sfd);
    return -1;
  }

  /* The server "MUST" close the data connection port when: 
   * "The port specification is changed by a command from the user".
   * Source: rfc 959 page 19 */
  if (session->d_sfd > 0) {
    if (close (session->d_sfd) == -1)
      fprintf (stderr, "%s: close: %s\n", __FUNCTION__, strerror (errno));
    session->d_sfd = 0;
  }

  /* Filter invalid PORT arguments by comparing the length of the argument. Too
   * many or too little number of characters in the string means that the
   * argument is invalid. */
  if ((cmd_str_len = strlen (cmd_str)) < (MIN_PORT_STR_LEN - 1)) {
    fprintf (stderr, "%s: PORT argument too short\n", __FUNCTION__);
    send_mesg_501 (session->c_sfd);
    return -1;
  } else if (cmd_str_len > (MAX_PORT_STR_LEN - 1)) {
    fprintf (stderr, "%s: PORT argument too long\n", __FUNCTION__);
    send_mesg_501 (session->c_sfd);
    return -1;
  }    

  //Convert the address h1,h2,h3,h4,p1,p2 into a hostname and service.
  if (get_port_address (session->c_sfd, &hostname, &service, cmd_str) == -1) {
    return -1;
  }
  char *portsuccess = "200 PORT command successful. Consider using PASV.\n";

  send_all(session->c_sfd,(uint8_t*)portsuccess,strlen(portsuccess));

  //Create a data connection to the hostname and service provided by the client.
  if ((session->d_sfd = port_connect (hostname, service)) == -1) {
    //Error message code to control socket.
    return -1;
  }
  
  return session->d_sfd;
}


/******************************************************************************
 * Convert the argument recieved with the PORT command to a hostname and
 * service string that can be used as arguments to getaddrinfo(). 
 *
 * The port command is entered as: PORT h1,h2,h3,h4,p1,p2 where h1-h4 are the
 * decimal values of each byte in the hostname and p1-p2 are the high and low
 * order bytes of the 16bit integer port.
 *
 * The argument 'cmd_str' is the command recieved by the server on the control
 * connection. The command string must be passed to this function in its
 * entirety.
 *
 * Arguments:
 *      c_sfd - The socket file descriptor for the control connection.
 *   address  - The address string, passed as a pointer to this function, will
 *              be set to the IPv4 dot notation address on function return.
 *   service  - The service string, passed as a pointer to this function, will
 *              be set to the port integer value expressed as a string on
 *              function return.
 *    cmd_str - The string of the port command. "PORT h1,h2,h3,h4,p1,p2\n"
 *
 * Return values:
 *   0    The hostname and service strings have been successfuly set.
 *  -1    Error, hostname and service strings are not set.
 *
 * Original author: Evan Myers
 *****************************************************************************/
int get_port_address (int c_sfd,
		      char (*address)[INET_ADDRSTRLEN], 
		      char (*service)[MAX_PORT_STR], 
		      char *cmd_str)
{
  /* Used to collect each byte of the hostname and port that was passed to the
   * function in the string argument 'cmd_str'.
   *
   * The bytes that will be collected from the command string and stored in the
   * elements of this array have been stored in a value that is larger than 8
   * bits. This has been done so that any byte value that is too large (an
   * invalid IPv4 address or port) can be found. */
  uint16_t h[PORT_BYTE_ARGS];    //h1,h2,h3,h4,p1,p2 see the function header.
  int h_index;

  int cmd_str_len;  //Length of the command string (function argument 3).
  uint16_t port;    //Stores the combined p1 and p2 value.
  int i;            //Loop counter.

  int char_counter; /* Used to ensure only one comma character is present 
		     * between each byte field in the command string. */
  
  //Initialize counters and compare values.
  cmd_str_len = strlen (cmd_str);
  char_counter = 0;
  h_index = 0;
  i = 0;

  /* Process all characters in the command string. This includes the command
   * portion "PORT " and the argument portion "h1,h2,h3,h4,p1,p2\n". Check for
   * errors in the argument string. */
  while (i < cmd_str_len) {
    /* Enter this block if the current character is not a digit. All characters
     * that are not digits (0-9) will be processed in this block, and the loop
     * will be restarted with the continue statement to process the next
     * character. */
    if ((cmd_str[i] < 48) || (cmd_str[i] > 57)) {
      //Only one non-digit character may appear in one continuous sequence.
      if (char_counter == 1) {
	fprintf (stderr, "%s: illegal character in argument\n", __FUNCTION__);
	send_mesg_501 (c_sfd);
	return -1;
      }

      //Check the expected character locations for the expected characters.
      if (h_index == 0) {
	//The argument string must begin with an integer.
	fprintf (stderr, "%s: illegal character in argument\n", __FUNCTION__);
	send_mesg_501 (c_sfd);
      } else if (h_index < 6) {
	//Only a comma may seperate each byte field.
	if (cmd_str[i] != ',') {
	  fprintf (stderr, "%s: illegal character in argument\n", __FUNCTION__);
	  send_mesg_501 (c_sfd);
	}
      } else {
	//The last character must be a null terminator.
	if (cmd_str[i] != '\0') {
	  fprintf (stderr, "%s: illegal character in argument\n", __FUNCTION__);
	  send_mesg_501 (c_sfd);
	}
      }
      //Increment counts on every character read.
      char_counter++;
      i++;
      continue;
    }


    /* Reset the count of continuous non-digit characters. The next character
     * is part of an integer. */
    char_counter = 0;

    /* When an integer is found, store the integer. See acknowledgement ONE in
     * the file header for the meaning of "SCNu16". */
    if (sscanf (cmd_str + i, "%"SCNu16, &h[h_index]) == -1) {
      if (errno == EINTR) {
	i--; //So that the same character will be passed to sscanf()
	continue;
      }
      fprintf (stderr, "%s: sscanf: %s\n", __FUNCTION__, strerror (errno));
      return -1;
    }

    //Determine if the integer is too large to be stored in 1-byte.
    if (h[h_index] > MAX_8_BIT) {
      fprintf (stderr, "%s: invalid PORT argument, %"PRIu16" is larger than"
	       " a byte\n", __FUNCTION__, h[h_index]);
      send_mesg_501 (c_sfd);
      return -1;
    }

    //Determine how many digits present in the integer.
    if (h[h_index] > 99) {
      i += 3;
    } else if (h[h_index] > 9) {
      i += 2;
    } else {
      i++;
    }

    //Store the next integer on the next iteration.
    h_index++;
  }

  //Ensure that the correct number of integers were present in the string.
  if (h_index < (PORT_BYTE_ARGS - 1)) {
    fprintf (stderr, "%s: improper PORT argument\n", __FUNCTION__);
    send_mesg_501 (c_sfd);
    return -1;
  }

  //Store the hostname in an IPv4 dot notation string. See acknowledgement ONE.
  sprintf (*address, "%"PRIu16".%"PRIu16".%"PRIu16".%"PRIu16,
	   h[0], h[1], h[2], h[3]);

  /* Multiply the value of the high order port byte by 256, to shift this byte
   * into the correct position. Works for big endian and little endian. */
  h[4] = (h[4] * 256);
  //Combine the two port bytes to create one integer.
  port = (h[4] | h[5]);
  //Store the integer as a string. See acknowledgement ONE in the file header.
  sprintf (*service, "%"PRIu16, port);

  //The hostname and service have been set, return from the function.
  return 0;
}
  

/******************************************************************************
 * Connect to the address and port specified in the arguments recieved with the
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
 *
 * Original author: Evan Myers
 *****************************************************************************/
int port_connect (char *hostname, char *service)
{
  struct addrinfo hints, *result;   //getaddrinfo()
  int sfd;         //the file descriptor of the data connection socket
  int gai;         //getaddrinfo() error string

  //set the gettaddrinfo() hints
  bzero (&hints, sizeof(hints));
  hints.ai_family = AF_INET;        //IPv4, four byte address
  hints.ai_socktype = SOCK_STREAM;  //the data connection is stream

  //Create the address information using the hostname and service.
  if ((gai = getaddrinfo (hostname, service, &hints, &result)) == -1) {
    fprintf (stderr, "%s: getaddrinfo: %s\n", __FUNCTION__, gai_strerror(gai));
    return -1;
  }

  //Create the socket with the values returned by getaddrinfo().
  if ((sfd = socket (result->ai_family,
		     result->ai_socktype,
		     result->ai_protocol)) == -1) {
    fprintf (stderr, "%s: socket: %s\n", __FUNCTION__, strerror (errno));
    return -1;
  }

  //Connect to the client.
  if (connect (sfd, result->ai_addr, result->ai_addrlen) == -1) {
    fprintf (stderr, "%s: connect: %s\n", __FUNCTION__, strerror (errno));
    return -1;
  }



  freeaddrinfo (result);  //Free the getaddrinfo() result.
  return sfd;             //Return the data connection socket file descriptor.
}


/******************************************************************************
 * send_all - see net.h
 *****************************************************************************/
int send_all (int sfd, uint8_t *mesg, int mesg_len)
{
  int nsent = 0;
  int to_send = mesg_len;

  while (to_send > 0) {
    if ((nsent = send (sfd, mesg, to_send, 0)) == -1) {
      if (errno == EINTR)
	continue;
      fprintf (stderr, "%s: %s\n", __FUNCTION__, strerror (errno));
      return -1;
    }
    //Update the number of bytes to send to the socket.
    to_send -= nsent;
    nsent = 0;
  }

  return 0;
}
