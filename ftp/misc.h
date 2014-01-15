/******************************************************************************
 * Students: Evan Myers, Justin Slind, Alex Tai, James Yoo
 * Course: CMPT-361
 * Assignment #3 - ftp server
 * File: misc.h
 * Date: November 2013
 *
 * Description:
 *   Miscellaneous ftp commands that don't fit in other categories at this
 *   time.
 *****************************************************************************/
#ifndef __MISC_H__
#define __MISC_H__

#include "session.h"

#define MAX_NUM_ARGS 1


/******************************************************************************
 * Changes Type. Valid types are ASCII Non-print and Binary.
 *
 * Arguments:
 *   si - info for current session
 *   arg - current command with parameter
 *
 * Original author: Justin Slind
 *****************************************************************************/
void cmd_type(session_info_t *si, char *arg);

/******************************************************************************
 * Changes Mode. Only valid mode is stream.
 *
 * Arguments:
 *   arg - current command with parameter
 *
 * Original author: Justin Slind
 *****************************************************************************/
void cmd_mode(session_info_t *si, char *arg);

/******************************************************************************
 * Send the system type on the control connection.
 *
 * Arguments:
 *   si - The control thread session information.
 *
 * Original author: Evan Myers
 *
 * Acknowledgements:
 *   http://cr.yp.to/ftp/syst.html
 *
 *   The author states "I recommend that all new servers respond to SYST with
 *   the meaningless string '215 UNIX Type: L8' exactly as used by the majority
 *   of current servers."
 *
 *   I have decided to follow this advice.
 *
 *   Also, ftp.gnu.org sends this message as a reply to the SYST command, and
 *   our server was created for a UNIX system.
 *****************************************************************************/
void cmd_syst (session_info_t *si);

/******************************************************************************
 * Set the file transfer structure.
 *
 * Arguments:
 *   si - The control thread session information.
 *  arg - The argument received from the client along with the STRU command.
 *
 * Original author: James Yoo
 *****************************************************************************/
void cmd_stru (session_info_t *si, char *arg, int argCount);


#endif //__MISC_H__
