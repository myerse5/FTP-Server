/******************************************************************************
 * Students: Evan Myers, Justin Slind, Alex Tai, James Yoo
 * Course: CMPT-361
 * Assignment #3 - ftp server
 * File: path.h
 * Date: November 2013
 *
 * Description:
 *   The functions on this determine if a pathname argument to a client command
 *   is acceptable for the given function and manipulate the pathname strings.
 *****************************************************************************/
#ifndef __PATH_H__
#define __PATH_H__


#include <stdbool.h> //Required for boolean return type in function prototype.


/******************************************************************************
 * Determine if a file exists. All components of the pathname must be present
 * on the filesystem. This function also checks that the pathname is for a file
 * that is either found in the server root directory, or is a descendant of the
 * server root directory.
 *
 * Arguments:
 *  cwd - The current working directory for this session.
 *  argpath - A pathname argument for a client command.
 *
 * Return values:
 *  true - The pathname is valid.
 *  false - The pathname is not valid.
 *
 * Commands which call this function: RETR
 *
 * Original author: Evan Myers
 *****************************************************************************/
bool check_file_exist (const char *cwd, char *argpath);


/******************************************************************************
 * Determine if a directory exists. All components of the pathname must be
 * present on the filesystem. All prefix components of the pathname must be
 * directories.
 * 
 * This function also checks that the pathname is for a file that is either
 * found in the server root directory, or is a descendant of the server root
 * directory. 
 *
 * The final component to the pathname must be a directory.
 *
 * Arguments:
 *  cwd - The current working directory for this session.
 *  argpath - A pathname argument for a client command.
 *
 * Return values:
 *  true - The pathname is valid.
 *  false - The pathname is not valid.
 *
 * Commands which call this function: NLST, LIST, CWD
 *
 * Original author: Evan Myers
 *****************************************************************************/
bool check_dir_exist (const char *cwd, char *argpath);


/******************************************************************************
 * Determine if a file may be created with the given pathname. All components
 * of the pathname must be present on the filesystem. All prefix components of
 * the pathname must be directories.
 *
 * This function also checks that the pathname is for a file that is either
 * found in the server root directory, or is a descendant of the server root
 * directory. 
 *
 * The final component of the filename must not be a directory.
 *
 * Arguments:
 *  cwd - The current working directory for this session.
 *  argpath - A pathname argument for a client command.
 *
 * Return values, when argument unique is false:
 *   0 - The pathname is valid.
 *  -1 - There was an error.
 *  -2 - The filename is not within the root directory (defined in ftp.conf)
 *
 * Additional return values when unique is true:
 *  -3 - The pathname is not unique.
 *
 * Commands which call this function: STOR, APPE, MKD, STRU
 *
 * Original author: Evan Myers
 *
 * note: argpath is not meant to be "const char *argpath"
 *****************************************************************************/
int check_futer_file (const char *cwd, char *argpath, bool unique);


/******************************************************************************
 * This function merges the three path fragments into a single pathname. This
 * value is used by a command function, eg. RETR, to find the pathname to the
 * requested file. 
 *
 * The locations and purpose of each pathname:
 *
 *   -rootdir: A global variable. Stores the path to the root directory
 *             selected by the server. This value is set in the file 'ftp.conf'.
 *
 *   -cwd:     The current working directory for the client session. This is a
 *             relative pathname to rootdir, and can be found in session_info_t.
 *
 *    argpath: This is the pathname supplied as an argument by the client. This
 *             string is relative to the current working directory found above.
 *             This value is discarded when a command thread no longer requires
 *             it.

 * Note: The string returned by this function was created with a call to malloc.
 *       The caller function must be sure to free this memory.
 *
 * Arguments:
 *      cwd - The current working directory string as described above.
 *
 *  argpath - The pathname argument received from the client as described above.
 *
 *  reserve - When checking if a file may be created on the server
 *            (eg. the STOR command), it is necessary to remove the filename
 *            that does not yet exist from the filepath before calling the
 *            function canonicalize_file_name(). In this case, the argpath
 *            string may be "trimmed" of the non-existant file before calling
 *            this function. The string returned by this function will contain
 *            "reserve" amount of indexes so the trimmed file may later be
 *            re-attached to the pathname.
 *
 *            NULL can be passed to this argument if no space needs to be
 *            reserved.
 *
 * Return value:
 *   string - The full pathname to the argument (rootdir -> cwd -> argument).
 *     NULL - Error
 *****************************************************************************/
char *merge_paths (const char *cwd, char *argpath, const int *reserve);


#endif //__PATH_H__
