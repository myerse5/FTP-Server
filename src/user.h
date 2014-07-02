/******************************************************************************
 * FTP-Server
 * Authors: Evan Myers, Justin Slind, Alex Tai, James Yoo
 * Date: November 2013
 *
 * Description:
 *   Functions to deal with users logging into the server.
 *****************************************************************************/
#ifndef __USER_H__
#define __USER_H__


/******************************************************************************
 * Logs the client onto the server with a username in preparation for the PASS
 * command.
 *
 * Arguments:
 *    si - Info for the current session.
 *   arg - The username argument.
 *****************************************************************************/
void cmd_user (session_info_t *si, char *arg);


/******************************************************************************
 * Finish the login procedure for the client. The PASS command must be sent to
 * the server immediately after the USER command. If the argument (arg)
 * passed to this function does not match the password found in 'users.conf'
 * for the given user, the login procedure will not complete and the client
 * must start again.
 *
 * Arguments:
 *    si - Info for the current session.
 *   arg - The password argument.
 *****************************************************************************/
void cmd_pass (session_info_t *si, char *cmd);


#endif //__USER_H__
