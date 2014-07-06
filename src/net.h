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
 ******************************************************************************/
#ifndef __NET_H__
#define __NET_H__


#include <arpa/inet.h> // required for the INETADDR_STRLEN in function prototype
#include <stdbool.h>   // required for 'bool' in function prototype
#include <stdint.h>    // required for 'uint8_t' in function prototype
#include "session.h"   // required for 'session_info_t' in function prototype


// The number of bits in a byte.
#define BITS_IN_BYTE 8


/******************************************************************************
 * accept_connection - constants
 *
 * These are the current modes that may be passed to function accept_connection
 * as the second argument. The mode argument to this function will determine
 * if standard input should be included in the select() read set, and if the
 * socket that was passed to accept() may be closed if accept() was successful. 
 *****************************************************************************/
#define ACCEPT_CONTROL 1    // Accept a control connection.
#define ACCEPT_PASV    2    // Accept a connection on a PASV socket.

// See "Other return values" in the accept_connection() function header.
#define STDIN_READY -999


/******************************************************************************
 * Create a socket to listen for connections from a new client. The socket
 * will be created on the interface that is found in the configuration file.
 * The port of the socket is chosen from the value in the configuration file.
 *
 * This function should only be used when creating a control connection socket.
 *
 * Return values:  
 *   >0   The socket file descriptor ready to accept() a control connection 
 *        any/all clients.
 *   -1   Error
 *****************************************************************************/
int get_control_sock (void);


/******************************************************************************
 * Accept a connection on the socket passed in the first argument. This
 * function can be used when accepting a control connection or a data
 * connection.
 *
 * modes (passed in the second argument):
 *    ACCEPT_CONTROL - 
 *        In this mode, standard input from the terminal that running the
 *        server will be added to the select() read set. By monitoring stdin,
 *        the function main() can idle in this function indefinitely while
 *        the server is running, until a new client wishes to connect to the
 *        server, or a server command is entered.
 *
 *        An example of a server command is "help" and "shutdown". For more
 *        information on all server commands, view the file "servercmd.h".
 *        Alternatively, run the server and type "help".
 *
 *    ACCEPT_PASV -
 *        The caller function should pass this mode when wishing to accept a
 *        data connection on a socket created with the PASV command. It is
 *        intended for only one connection to be accepted on a passive socket.
 *        When this mode is selected, if a connection is accepted, the listening
 *        socket created in the PASV command will be closed before returning
 *        from this function.
 *
 *        This mode should be selected when the function caller is a command
 *        thread.
 *
 * Arguments:
 *    sfd - Accept a connection with this socket.
 *
 *   mode - Modify the actions performed by this function. See modes above.
 *
 *     si - A pointer to the session information. If a command thread is
 *          running this function, the variable cmd_quit will be checked
 *          periodically while accepting a connection. If the thread creator
 *          wishes the thread to terminate, the thread creator will modify
 *          this value.
 *
 *          When the function caller is not a command thread, NULL should  be
 *          passed in this argument. It will not be checked if mode was set
 *          appropriately.
 *
 * Return values:
 *   >0   The socket file descriptor of the newly created control connection.
 *   -1   Error, the connection could not be established with the client.
 *
 * Other return values:
 *   STDIN_READY - 
 *      This value may be returned when the function passed the mode
 *      ACCEPT_CONTROL as the second argument. This value is returned when
 *      there is input to read on stdin. 
 *****************************************************************************/
int accept_connection (int sfd, int mode, session_info_t *si);


/******************************************************************************
 * Create a TCP data connection socket for the PASV server command. 
 *
 * The interface that this socket is created on will be the value which is set
 * for the INTERFACE_CONFIG setting in the configuration file.
 *
 * Send the address information of the newly created socket to the client over
 * the control connection. The newly created listening socket file descriptor
 * will be stored in the session_info_t structure when the function does not
 * return error.
 *
 * Arguments:
 *   si - Contains the session information of the control thread.
 *
 * Return values:
 *  >0    The socket file descriptor of the data connection socket.
 *  -1    Error, the socket could not be created.
 *****************************************************************************/
int cmd_pasv (session_info_t *si);


/******************************************************************************
 * Find the address of an interface.
 *
 * Set the string pointer passed as argument two of this function to the dot
 * notation (eg. 127.0.1.1) IPv4 address of the interface passed as the first
 * argument to this function.
 *
 * Arguments:
 *   interface - Retrieve the address of this interface.
 *   address   - A string pointer to be set to the retrieved interface address
 *               on function return.
 *
 * Return values:
 *   0    Success, the interface was found and the address of that interface
 *        set in the string passed in the second argument.
 *        argument.
 *  -1    Error, the address has not been set in the second argument. 
 *****************************************************************************/
int get_interface_address (const char *interface,
			   char (*address)[INET_ADDRSTRLEN]);


/******************************************************************************
 * Filter the PORT command argument, storing the connection information in the
 * session_info_t structure if the argument is accepted.
 *
 * Arguments:
 *    si  - Contains the session information of the control thread.
 *   arg  - The port argument string "h1,h2,h3,h4,p1,p2"
 *
 * Return values:
 *    0   The PORT command was accepted.
 *   -1   The PORT command was rejected.
 *****************************************************************************/
int cmd_port (session_info_t *si, char *arg);


/******************************************************************************
 * Connect to the address and port specified in the arguments received with the
 * PORT command.
 *
 * Arguments:
 *   hostname - The IPv4 address to connect to, represented in a dot notation
 *              string. (eg. "127.0.1.1")
 *       port - The port to connect to, represented as a string. (eg. "56035")
 *
 * Return values:
 *   >0    The socket file descriptor of the newly created data connection.
 *   -1    Error, the connection could not be made.
 *****************************************************************************/
int port_connect (char *hostname, char *port);


/******************************************************************************
 * Send the entire message found in the second argument to the socket passed
 * in the first argument. This function was created to handle partial sends.
 *
 * Arguments:
 *    sfd - The socket file descriptor to send the message to.
 *   mesg - The message to send.
 * toSend - The size of the message.
 *
 * Return values:
 *   0    The full message was successfully sent.
 *  -1    Error, the message was not sent in full.
 *****************************************************************************/
int send_all (int sfd, uint8_t *mesg, int toSend);


#endif // __NET_H__
