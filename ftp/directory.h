/******************************************************************************
 * Students: Evan Myers, Justin Slind, Alex Tai, James Yoo
 * Course: CMPT-361
 * Assignment #2 - ftp server
 * Files: directory.h
 * Date: November 2013
 * 
 * Description:
 *   FTP functions that create or change directories.
 *****************************************************************************/
#ifndef __DIRECTORY_H__
#define __DIRECTORY_H__


#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include "session.h" //Required for session_info_t in function prototype.


/******************************************************************************
 * Send the files in a directory over a data connection.
 *
 * This function handles the communication on the control connection. It will
 * call listDirect() (a static helper function) to send the files in the
 * directory on the data connection.
 *
 * Arguments: 
 *        si - A pointer to the session information maintained by session().
 *   argpath - The filename argument received with the list or nlist command.
 *    detail - Set to true if a detailed listing was requested.
 *
 * Original author: Alex Tai
 * Rewritten by: Evan Myers
 *****************************************************************************/
void cmd_list_nlst (session_info_t *si, char *argpath, bool detail);


/******************************************************************************
 * Create a directory.
 *
 * Arguments:
 *        si - A pointer to the session information maintained by session().
 *  filepath - The filename argument supplied with the mkdir command received
 *             from the client.
 * 
 * Original author: Alex Tai
 * Checked by: Evan Myers
 *****************************************************************************/
void makeDir (session_info_t *si, char *filepath);


/******************************************************************************
 * Change the current working directory to the parent directory by calling
 * cmd_cwd() with the argument path "..".
 *
 * Argument:
 *   si  - The command thread session information.
 *   arg - The command is rejected if this argument is not NULL.
 * 
 * Original author: Evan Myers
 * Checked by: Alex Tai
 *****************************************************************************/
void cmd_cdup (session_info_t *si, char *arg);


/******************************************************************************
 * Change the current working directory.
 *
 * Argument:
 *   si  - The command thread session information.
 *   arg - Change the current working directory to this path if accepted.
 * 
 * Original author: Alex tai
 * Changed by: Evan Myers - using code written by James Yoo and Justin Slind
 *****************************************************************************/
void cmd_cwd (session_info_t *si, char *arg);


/******************************************************************************
 * Send the current working directory to the client.
 *
 * Argument:
 *   si - The command thread session information.
 *
 * Original author: Evan Myers
 *****************************************************************************/
void cmd_pwd (session_info_t *si);


#endif //__DIRECTORY_H__
