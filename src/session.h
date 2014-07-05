/******************************************************************************
 * FTP-Server
 * Authors: Evan Myers, Justin Slind, Alex Tai, James Yoo
 * Date: November 2013
 *
 * Description:
 *   The main loop that each client goes through.  Accepts commands and stores
 *   them in a queue.  Launches a separate thread to deal with commands one at
 *   a time. Handles the abort.
 *****************************************************************************/
#ifndef __SESSION_H__
#define __SESSION_H__


#include <arpa/inet.h>  // Required for INETADDR_STRLEN in structure.
#include <stdbool.h>    // Required for 'bool' in structure.
                       

/* TODO: Remove these buffer overruns, tie them all together and define one
 *       one (hopefully) constant in an appropriate place. */
#define CMD_STRLEN   4096
#define USER_STRLEN  64
#define ABORT_STRLEN 5


/******************************************************************************
 * Timeout values for the control thread and command thread.
 *****************************************************************************/
#define SERVER_SHUTDOWN_TIMEOUT_SEC 0
#define SERVER_SHUTDOWN_TIMEOUT_USEC 100000

#define COM_THREAD_ABORT_TIMEOUT_SEC 0
#define COM_THREAD_ABORT_TIMEOUT_USEC 100000


/******************************************************************************
 * String size constants used for for a legal PORT argument. These values are
 * used by the session_info_t structure found in 'session.h', and the functions
 * found in 'net.c'.
 *****************************************************************************/
/* The maximum string length of the arguments to the PORT command.
 * (6 three digit fields) + (5 commas) + (null terminator) */
#define MAX_CMDPORT_ARG_STRLEN ((6*3) + 5 + 1)

/* The minimum string length of the arguments to the PORT comman.
 * (PORT + space) + (6 one digit fields) + (5 commas) + (newline + null) */
#define MIN_CMDPORT_ARG_STRLEN ((6*1) + 5 + 1)

/* The maximum length of a port integer when expressed as a string.
 * (((2^16) - 1) = 65535) == (5 chars + 1 for the NULL terminator) == 6 chars */
#define MAX_PORT_INT_STRLEN 6


/******************************************************************************
 * This structure holds the connection info that will be used to establish the
 * next data connection with the client. When the data connection has been
 * established, these variables must be set as follows:
 *
 *     hostname: The first character MUST be set to '\0'.
 *         port: The first character MUST be set to '\0'.
 *         pasv: MUST be set to 0.
 *
 * After a successful PASV command (the command is accepted), the structure
 * variables must be set as follows:
 *
 *     hostname: Set to the dot-notation ip address sent in the PORT request.
 *               e.g. "xxx.xxx.x.xxx"
 *         port: Set to the integer port that was sent in the PORT request,
 *               represented as a string. e.g. "22231"
 *         pasv: MUST be set to 0.
 *
 * After a successful PASV command (the command is accepted), the structure
 * variables must be set as follows:
 *
 *     hostname: The first character MUST be set to '\0'.
 *         port: The first character MUST be set to '\0'.
 *         pasv: Set to the socket file descriptor of the newly created
 *               listening socket.
 *****************************************************************************/
typedef struct {
  char hostname[INET_ADDRSTRLEN];
  char port[MAX_PORT_INT_STRLEN];
  int pasv;
} conn_info_t;


/******************************************************************************
 * The session info structure. Exactly one of these structures is created for
 * each control thread, or in other words, for each connected client. It is
 * created by the function session(), and a pointer to this structure is passed
 * as an argument to each command thread.
 * 
 * This structure contains any socket information, the command and arguments
 * received with the command from the client, and other useful information that
 * is required for a command thread to perform the required action and
 * communicate with the client.
 *
 * A command thread and the command thread which created it communicate by
 * changing values in this structure (ie. control thread sets abort to true,
 * the command thread notices this change and terminates).
 *
 * session_info_t
 *    |
 *    |----cwd
 *    |----user
 *    |----loggedin
 *    |----cmdComplete
 *    |----cmdAbort
 *    |----type
 *    |----csfd
 *    |----dsfd
 *    |----connInfo
 *            |
 *            |----hostname
 *            |----port
 *            |----pasv
 *****************************************************************************/
typedef struct {
  char cwd[CMD_STRLEN];		// The current working directory.
  char user[USER_STRLEN];	// The current username.
  bool loggedin;		// True if the client has completed the login.
  bool cmdComplete;		// True when the command thread is complete.
  bool cmdAbort;		// True requests the command thread to abort.
  char type;                    // The data connection transfer type.
  int csfd;	        	// The control connection socket file descriptor
  int dsfd;		      	// The data connection socket file descriptor.

  /* Contains the service and hostname required to establish PORT connection,
   * or a listening socket created with the PASV command. */
  conn_info_t connInfo;

  // A string containing the next command to process in this session.
  char cmdString[CMD_STRLEN];	

  // A request to clean up and return from the session has been given by main().
  bool cmdQuit;	         	
} session_info_t;


/******************************************************************************
 * This function can be viewed as the main() function for each control thread.
 * session() will be running for every control connection. Commands sent by
 * the client are read from the control connection, and a new thread (referred
 * to as a control thread) is created to process the command.
 *
 * While a control thread is still active, session() will continue to read
 * commands sent over the control connection socket. A command thread is
 * created to handle a command received from the client. Only one command
 * thread is created by session() at one time.
 *
 * When a command is received from the client, and the previous command thread
 * has not yet been joined (terminated), any command that is not immediately
 * required to be handled will be added to a command queue. The commands quit
 * and abort are processed immediately (by changing a variable that the
 * command thread checks periodically, and terminating if appropriate).
 *
 * session() monitors a global variable that is set by main to determine if the
 * program should terminate. When this variable (shutdown) has been set,
 * session() will signal the command thread to terminate. Before this function
 * returns, it will join with the command thread (to ensure the command thread
 * has cleaned up and sent the appropriate messages to the client).
 *
 * Arguments:
 *   csfd - a control connection socket.
 *
 * Return values:
 *    0   success
 *   -1   error
 *****************************************************************************/
int session (int csfd);


/******************************************************************************
 * Read a command from the control connection.
 *
 * Arguments:
 *    str - A string to be set to the received command.
 *   sock - A control connection socket file descriptor.
 *     si - The control thread session information.
 *
 * Return values:
 *    0   success
 *   -1   error
 *****************************************************************************/
int read_cmd (char *str, int sock, session_info_t *si);


#endif // __SESSION_H__
