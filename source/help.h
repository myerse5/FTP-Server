/******************************************************************************
 * Authors: Evan Myers, Justin Slind, Alex Tai, James Yoo
 * FTP-Server
 * Date: November 2013
 *
 * Description:
 *   The help function is found in this file.
 *****************************************************************************/
#ifndef __HELP_H__
#define __HELP_H__


#include "session.h" //Required for session_info_t in function prototype.


/******************************************************************************
 * When HELP is invoked without an argument, it will return a list of all the
 * available FTP server commands that the client is able to use.
 *
 * When HELP is invoked with an argument, it will return a short description
 * of the specified command and how it is to be used.
 *
 * Arguments:
 *   si - Contains the session information for the current client.
 *  arg - The command that help is requested for. This value may be NULL in
 *        which case a list of commands will be printed.
 *
 * Original author: James Yoo
 *****************************************************************************/
int command_help (session_info_t *si, char *arg);


#endif  //__HELP_H__
