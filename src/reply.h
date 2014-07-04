/******************************************************************************
 * FTP-Server
 * Authors: Evan Myers, Justin Slind, Alex Tai, James Yoo
 * Date: November 2013
 *
 * Description:
 *    All response messages sent to the client can be found in this file. All
 *    communication sent by the server to the client on the control connection
 *    is sent by a function found in this file.
 *
 *    This has been done to ensure all communication is consistent, and to
 *    make modififying these responses easier.
 *
 * Regarding documentation of this file:
 *    Most of the functions in this file contain the same arguments and return
 *    values. They will be ommitted so as not to become trivial and repetitive.
 *    The missing information can be found directly below.
 *
 * Arguments:
 *   csfd - The file descriptor of the control connection socket. The response
 *           message will be sent to this socket.
 *
 * Return values:
 *    0   The message was successfully sent to the socket.
 *   -1   The entire message was not sent to the socket.
 *****************************************************************************/
#ifndef __REPLY_H__
#define __REPLY_H__


#define REPLY_150_ASCII   'a'
#define REPLY_150_BINARY  'b'

#define REPLY_200_PORT    'p'
#define REPLY_200_ASCII   'a'
#define REPLY_200_IMAGE   'i'
#define REPLY_200_STREAM  's'
#define REPLY_200_FSTRU   'f'

#define REPLY_226_ABORT   'a'
#define REPLY_226_SUCCESS 's'

#define REPLY_230_NONEED  'n'
#define REPLY_230_SUCCESS 's'

#define REPLY_530_REQUEST 'r'
#define REPLY_530_FAIL    'f'


/******************************************************************************
 * A positive message including the type of transfer (BINARY or ASCII), and the
 * name of the file being transferred.
 *****************************************************************************/
int send_mesg_150 (int csfd, const char *filename, char option);


/******************************************************************************
 * Generate a "command okay" message to the client, with the specific content
 * controlled by the option.
 *
 * option: REPLY_200_PORT   - port command successful
 *         REPLY_200_ASCII  - switching to ascii mode
 *         REPLY_200_IMAGE  - switching to image mode
 *         REPLY_200_STREAM - switching to stream mode
 *         REPLY_200_FSTRU  - switching to File structure
 *****************************************************************************/
int send_mesg_200 (int csfd, char option);


/******************************************************************************
 * Send the welcome message.
 *****************************************************************************/
int send_mesg_215 (int csfd);


/******************************************************************************
 * Send the welcome message to a newly connected client. This function should
 * be called after a control connection has been established with a client.
 *****************************************************************************/
int send_mesg_220 (int csfd);


/******************************************************************************
 * Send a help message that lists all implemented commands on the server.
 *****************************************************************************/
int send_mesg_214_general (int csfd);


/******************************************************************************
 * Send a help message that describes the syntax and purpose of a command.
 *
 * Arguments:
 *     csfd - The control socket file descriptor to send the message to.
 *   syntax - Usage instructions for the requested command.
 *     info - An explanation for what the command does.
 *****************************************************************************/
int send_mesg_214_specific (int csfd, char *syntax, char *info);


/******************************************************************************
 * Sends the goodbye message.
 *****************************************************************************/
int send_mesg_221 (int csfd);


/******************************************************************************
 * Positive response. The requested file action was successful.
 *
 * option: REPLY_226_ABORT   - abort successful
 *         REPLY_226_SUCCESS - closing data connection, file action successful
 *****************************************************************************/
int send_mesg_226 (int csfd, char option);


/******************************************************************************
 * Generates a positive response message for the PASV command as specified in
 * 'RFC 959'.
 *
 * Create the PASV command response message, then send the feedback to the
 * client on the control connection. The response will be sent in the following
 * format: "Entering Passive Mode (h1,h2,h3,h4,p1,p2).\n".
 *
 * NOTE: The response message is sent to the control connection socket (c_sfd),
 *       and this function generates this message using the data creation
 *       socket (d_sfd). Both are required for this function.
 *
 * Arguments:
 *   csfd - The file descriptor of the control connection socket. The response
 *          message will be sent to this socket.
 *   dsfd - The file descriptor of the data connection socket. The address
 *          information sent to the control connection socket will be created
 *          with this socket.
 *
 * Return values:
 *    0   The message was successfully sent to the socket.
 *   -1   The entire message was not sent to the socket.
 *****************************************************************************/
int send_mesg_227 (int csfd, int dsfd);


/******************************************************************************
 * A positive response for a successful login.
 *
 * option: REPLY_230_NONEED  - already logged in
 *         REPLY_230_SUCCESS - login successful
 *****************************************************************************/
int send_mesg_230 (int csfd, char option);


/******************************************************************************
 * A positive response for when a directory change was successful.
 *****************************************************************************/
int send_mesg_250 (int csfd);


/******************************************************************************
 * A positive response message displaying a directory name. To be used with
 * the MKD and PWD commands.
 *****************************************************************************/
int send_mesg_257 (int csfd, const char *mesg);


/******************************************************************************
 * A positive response after accepting a username. Request the PASS command to
 * follow with the client's password.
 *****************************************************************************/
int send_mesg_331 (int csfd);


/******************************************************************************
 * A temporary negative response. The data connection cannot be established.
 *****************************************************************************/
int send_mesg_425 (int csfd);


/******************************************************************************
 * A temporary negative response. The data connection was closed prematurely.
 *****************************************************************************/
int send_mesg_426 (int csfd);


/******************************************************************************
 * A permanent negative response. Action not taken because the file is
 * unavailable.
 *****************************************************************************/
int send_mesg_450 (int csfd);


/******************************************************************************
 * A negative response sent when there was an error in processing.
 *****************************************************************************/
int send_mesg_451 (int csfd);


/******************************************************************************
 * Generates a negative response message for when the command was not found.
 *****************************************************************************/
int send_mesg_500 (int csfd);


/******************************************************************************
 * Generates a negative response message for when errors have been detected in
 * the argument of a command.
 *****************************************************************************/
int send_mesg_501 (int csfd);


/******************************************************************************
 * Negative response when the client attempts to give a password without
 * supplying a username first.
 *****************************************************************************/
int send_mesg_503 (int csfd);


/******************************************************************************
 * Generates a negative response message for when a command has not been
 * implemented.
 *****************************************************************************/
int send_mesg_504 (int csfd);


/******************************************************************************
 * Generates a negative response message for when the client has entered a
 * command which requires the client to be logged in, but this requirement has
 * not been met or a password was not correct.
 *
 * option: REPLY_530_REQUEST - request the user login first
 *         REPLY_530_FAIL    - login failed
 *****************************************************************************/
int send_mesg_530 (int csfd, char option);


/******************************************************************************
 * Generates a permanent negative response for when a user gives an argument
 * to a command which accepts no arguments.
 *****************************************************************************/
int send_mesg_550_no_argument (int csfd);


/******************************************************************************
 * Generates a permanent negative response when a directory was not found.
 *****************************************************************************/
int send_mesg_550_no_dir (int csfd);


/******************************************************************************
 * Generates a temporary negative response message when the server encounters
 * a processing error.
 *****************************************************************************/
int send_mesg_550_process_error (int csfd);


/******************************************************************************
 * Generates a negative response message for when a file cannot be processed
 * due to a lack of permissions.
 *****************************************************************************/
int send_mesg_550_unavailable (int csfd);


/******************************************************************************
 * Generates a negative response message for when the pathname of a file
 * argument is not allowed.
 *****************************************************************************/
int send_mesg_553 (int csfd);


#endif // __REPLY_H__
