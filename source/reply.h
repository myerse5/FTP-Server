/******************************************************************************
 * Authors: Evan Myers, Justin Slind, Alex Tai, James Yoo
 * FTP-Server
 * Date: November 2013
 *
 * Description:
 *    The functions found in this file send a response message to the client
 *    over the control connection socket. They were created to remove
 *    repetition in places where a response message is sent. Also, this will
 *    help ensure that all messages that are meant to send the same data will
 *    do so.
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


/******************************************************************************
 * Send the welcome message to a newly connected client. This function should
 * be called after a control connection has been established with a client.
 *****************************************************************************/
int send_welcome_mesg_220 (int csfd);


/******************************************************************************
 * This function generates a positive response message for the PASV command
 * as specified in 'RFC 959'.
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
 *
 * Original author: Evan Myers
 *****************************************************************************/
int send_mesg_227 (int csfd, int dsfd);


/******************************************************************************
 * A permanent negative response. Action not taken because file is unavailable.
 *****************************************************************************/
int send_mesg_450 (int csfd);

/******************************************************************************
 * A negative response sent when there was an error in processing.
 *****************************************************************************/
int send_mesg_451 (int csfd);


/******************************************************************************
 * This function generates a negative response message for when the command
 *****************************************************************************/
int send_mesg_500 (int csfd);


/******************************************************************************
 * This function generates a negative response message for when errors have
 * been detected in the argument of a command.
 *****************************************************************************/
int send_mesg_501 (int csfd);


/******************************************************************************
 * This function generates a negative response message for when the client has
 * entered a command which requires the client to be logged in, but this
 * requirement has not been met.
 *****************************************************************************/
int send_mesg_530 (int csfd);


/******************************************************************************
 * This function generates a negative response message for when a file cannot
 * be processed due to a lack of permissions.
 *****************************************************************************/
int send_mesg_550 (int csfd);


/******************************************************************************
 * This function generates a negative response message for when the pathname
 * of a file argument is not allowed.
 *****************************************************************************/
int send_mesg_553 (int csfd);


#endif //__REPLY_H__
