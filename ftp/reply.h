/******************************************************************************
 * Students: Evan Myers, Justin Slind, Alex Tai, James Yoo
 * Course: CMPT-361
 * Assignment #3 - ftp server
 * File: reply.h
 * Date: November 2013
 *
 * Description:
 *    The functions found in this file send a response message to the client
 *    over the control connection socket. They were created to remove
 *    repitition in places where a response message is sent. Also, this will
 *    help ensure that all messages that are meant to send the same data will
 *    do so.
 *****************************************************************************/
#ifndef __REPLY_H__
#define __REPLY_H__


/******************************************************************************
 * Send the welcome message to a newly connected client. This function should
 * be called after a control connection has been established with a client.
 *
 * Arguments:
 *   c_sfd - The file descriptor of the control connection socket. The response
 *           message will be sent to this socket.
 *
 * Return values:
 *    0   The message was successfuly sent to the socket.
 *   -1   The entire message was not sent to the socket.
 *
 * Original author: Evan Myers
 *****************************************************************************/
int send_welcome_mesg_220 (int c_sfd);


/******************************************************************************
 * This function generates a posotive response message for the PASV command
 * as specified in 'rfc 959'.
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
 *   c_sfd - The file descriptor of the control connection socket. The response
 *           message will be sent to this socket.
 *   d_sfd - The file descriptor of the data connection socket. The address
 *           information sent to the control connection socket will be created
 *           with this socket.
 *
 * Return values:
 *    0   The message was successfuly sent to the socket.
 *   -1   The entire message was not sent to the socket.
 *
 * Original author: Evan Myers
 *****************************************************************************/
int send_mesg_227 (int c_sfd, int d_sfd);


/******************************************************************************
 * A permanent negative response. Action not taken because file is unavailable.
 *
 * Arguments:
 *   c_sfd - The file descriptor of the control connection socket. The response
 *           message will be sent to this socket.
 *
 * Return values:
 *    0   The message was successfuly sent to the socket.
 *   -1   The entire message was not sent to the socket.
 *
 * Original author: Evan Myers
 *****************************************************************************/
int send_mesg_450 (int c_sfd);

/******************************************************************************
 * A negative response sent when there was an error in processing.
 *
 * Arguments:
 *   c_sfd - The file descriptor of the control connection socket. The response
 *           message will be sent to this socket.
 *
 * Return values:
 *    0   The message was successfuly sent to the socket.
 *   -1   The entire message was not sent to the socket.
 *
 * Original author: Evan Myers
 *****************************************************************************/
int send_mesg_451 (int c_sfd);


/******************************************************************************
 * This function generates a negative response message for when the command
 * was not recognized.
 *
 * Arguments:
 *   c_sfd - The file descriptor of the control connection socket. The response
 *           message will be sent to this socket.
 *
 * Return values:
 *   0    The message was successfuly sent to the socket.
 *  -1    The entire message was not sent to the socket.
 *
 * Original author: Evan Myers
 *****************************************************************************/
int send_mesg_500 (int c_sfd);


/******************************************************************************
 * This function generates a negative response message for when errors have
 * been detected in the argument of a command.
 *
 * Arguments:
 *   c_sfd - The file descriptor of the control connection socket. The response
 *           message will be sent to this socket.
 *
 * Return values:
 *   0    The message was successfuly sent to the socket.
 *  -1    The entire message was not sent to the socket.
 *
 * Original author: Evan Myers
 *****************************************************************************/
int send_mesg_501 (int c_sfd);


/******************************************************************************
 * This function generates a negative response message for when the client has
 * entered a command which requires the client to be logged in, but this
 * requirement has not been met.
 *
 * Arguments:
 *   c_sfd - The file descriptor of the control connection socket. The response
 *           message will be sent to this socket.
 *
 * Return values:
 *   0    The message was successfuly sent to the socket.
 *  -1    The entire message was not sent to the socket.
 *
 * Original author: Evan Myers
 *****************************************************************************/
int send_mesg_530 (int c_sfd);


/******************************************************************************
 * This function generates a negative response message for when a file cannot
 * be processed due to a lack of permissions.
 *
 * Arguments:
 *   c_sfd - The file descriptor of the control connection socket. The response
 *           message will be sent to this socket.
 *
 * Return values:
 *   0    The message was successfuly sent to the socket.
 *  -1    The entire message was not sent to the socket.
 *
 * Original author: Evan Myers
 *****************************************************************************/
int send_mesg_550 (int c_sfd);


/******************************************************************************
 * This function generates a negative response message for when the pathname
 * of a file argument is not allowed.
 *
 * Arguments:
 *   c_sfd - The file descriptor of the control connection socket. The response
 *           message will be sent to this socket.
 *
 * Return values:
 *   0    The message was successfuly sent to the socket.
 *  -1    The entire message was not sent to the socket.
 *
 * Original author: Evan Myers
 *****************************************************************************/
int send_mesg_553 (int c_sfd);


#endif //__REPLY_H__
