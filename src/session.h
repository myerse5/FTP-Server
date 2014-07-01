/******************************************************************************
 * Authors: Evan Myers, Justin Slind, Alex Tai, James Yoo
 * FTP-Server
 * Date: November 2013
 *
 * Description:
 *   The main loop that each client goes through.  Accepts commands and stores
 *   them in a queue.  Launches a separate thread to deal with commands one at
 *   a time. Handles the abort.
 *****************************************************************************/
#ifndef __SESSION_H__
#define __SESSION_H__


#include <stdbool.h>  //Required for 'bool' in structure.


//TODO update these random, arbitrary values.
#define CMD_STRLEN 4096
#define USER_STRLEN 64
#define ABORT_STRLEN 5


/******************************************************************************
 * Timeout values for the control thread and command thread.
 *****************************************************************************/
#define SERVER_SHUTDOWN_TIMEOUT_SEC 0
#define SERVER_SHUTDOWN_TIMEOUT_USEC 100000

#define COM_THREAD_ABORT_TIMEOUT_SEC 0
#define COM_THREAD_ABORT_TIMEOUT_USEC 100000


/******************************************************************************
 * The session info structure. Exactly one of these structures is created for
 * each control thread. It is created by the function session(), and a
 * pointer to this structure is passed as an argument to each command thread.
 * 
 * This structure contains any socket information, the command and arguments
 * received with the command from the client, and other useful information that
 * is required for a command thread to perform the required action and
 * communicate with the client.
 *
 * A command thread and the command thread which created it communicate by
 * changing values in this structure (ie. control thread sets abort to true,
 * the command thread notices this change and terminates).
 *****************************************************************************/
typedef struct {
  int csfd;	        	//control socket, rx from main
  int dsfd;		      	//data socket, created from command thread
  char cwd[CMD_STRLEN];		//current working directory
  char user[USER_STRLEN];	//username
  bool loggedin;		//whether user is logged in
  bool cmdComplete;		//command thread is complete
  bool cmdAbort;		//command to abort
  bool cmdQuit;	         	//command to quit has been given
  char cmdString[CMD_STRLEN];	//command string for current command
  char type;
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
 *
 * Original author: Justin Slind
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
 *
 * Original author: Justin Slind
 *****************************************************************************/
int read_cmd (char *str, int sock, session_info_t *si);


#endif //__SESSION_H__
