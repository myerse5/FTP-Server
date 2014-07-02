/******************************************************************************
 * FTP-Server
 * Authors: Evan Myers, Justin Slind, Alex Tai, James Yoo
 * Date: November 2013
 *
 * Description:
 *   Miscellaneous ftp commands that don't fit in other categories at this
 *   time.
 *****************************************************************************/
#ifndef __MISC_H__
#define __MISC_H__


#include "session.h" //Required for session_info_t in function parameters.


/******************************************************************************
 * Changes Type. Valid types are ASCII Non-print and Binary.
 *
 * Arguments:
 *   si - info for current session
 *   arg - current command with parameter
 *****************************************************************************/
void cmd_type (session_info_t *si, char *arg);


/******************************************************************************
 * Changes Mode. Only valid mode is stream.
 *
 * Arguments:
 *   arg - current command with parameter
 *****************************************************************************/
void cmd_mode (session_info_t *si, char *arg);


/******************************************************************************
 * Send the system type on the control connection.
 *
 * Arguments:
 *   si - The control thread session information.
 *****************************************************************************/
void cmd_syst (session_info_t *si);


/******************************************************************************
 * Set the file transfer structure.
 *
 * Arguments:
 *       si - The control thread session information.
 *      arg - The argument received from the client along with the STRU command.
 *  numArgs - The number of arguments to the command.
 *****************************************************************************/
void cmd_stru (session_info_t *si, char *arg, int numArgs);


/******************************************************************************
 * Quit the current session.
 *
 * Arguments:
 *   si - The control thread session information.
 *****************************************************************************/
void cmd_quit (session_info_t *si);


#endif //__MISC_H__
