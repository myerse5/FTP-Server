/******************************************************************************
 * FTP-Server
 * Authors: Evan Myers, Justin Slind, Alex Tai, James Yoo
 * Date: November 2013
 *
 * Description:
 *   FTP commands that deal with transfering files with the client: STOR, APPE,
 *   STOU, and RETR.
 *****************************************************************************/
#ifndef __TRANSFER_H__
#define __TRANSFER_H__


#include "session.h" //Required for session_info_t in function prototype.


/******************************************************************************
 * Stores a file with a unique filename.  Ignores parameter.
 *
 * Arguments:
 *    si - info for current session
 *   arg - The filename argument.
 *
 * Note: This function will store the unique name to the current working
 *       directory and not any other path argument. This is intended, we were
 *       following "RFC 959".
 *****************************************************************************/
void cmd_stou (session_info_t *si, char *arg);

/******************************************************************************
 * Stores a file on the server file system.
 *
 * Arguments:
 *    si - Info for current session.
 *   cmd - The filename argument.
 *****************************************************************************/
void cmd_stor (session_info_t *si, char *cmd);

/******************************************************************************
 * Appends a file to another file found on the server file system.
 *
 * Arguments:
 *    si - info for current session
 *   cmd - current command with parameter
 *****************************************************************************/
void cmd_appe (session_info_t *si, char *cmd);

/******************************************************************************
 * Retrieve a file from the server file system.
 *
 * Arguments:
 *     si - The session information.
 *   path - An absolute path, or relative filename to be retrieved.
 *****************************************************************************/
void cmd_retr (session_info_t *si, char *path);

/******************************************************************************
 * Close all sockets, reset the stored data socket file descriptor in the
 * session_info_t structure, and close the file pointer when appropriate.
 *
 * This function was created to help defend against programmer error. These
 * closing statements appear in many places in the stor command.
 *
 * Arguments:
 *   si - Info for the current session.
 *   fp - The open filestream, set this to NULL if no filestream has been
 *        opened.
 *   errcode - The type of error. This should be set to zero if no error
 *             has occurred (a clean exit from the caller function).
 *****************************************************************************/
void cleanup_stor_recv (session_info_t *si, FILE *fp, int errcode);


#endif //__TRANSFER_H__
