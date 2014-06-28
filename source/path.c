/******************************************************************************
 * Students: Evan Myers, Justin Slind, Alex Tai, James Yoo
 * Course: CMPT-361
 * Assignment #3 - ftp server
 * File: path.c
 * Date: November 2013
 *
 * Description:
 *   The functions on this determine if a pathname argument to a client command
 *   is acceptable for the given function and manipulate the pathname strings.
 *
 * Acknowledgements:
 *   We wanted to create a function that determines if a pathname argument for
 *   a command received from a client was appropriate.
 *
 *   One of the deciding factors to determine if the pathname is appropriate
 *   was to ensure that the pathname argument is a descendant of our servers
 *   chosen root directory.
 *
 *   When researching realpath() and PATH_MAX we determined that at this point
 *   in time realpath() is not safe or not portable.
 *
 *   We created a function to canonicalize a pathname, but struggled to resolve
 *   symbolic links.
 *
 *   While researching a solution to resolve symbolic links, we found this
 *   webpage:
 *   http://www.gnu.org/software/libc/manual/html_node/Symbolic-Links.html
 *
 *   The function canonicalize_file_name() is listed here, which we have
 *   used. It was the solution to the original problem. 
 *****************************************************************************/
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "path.h"
#include "reply.h"
#include "session.h"


extern char *rootdir;  //defined in the file 'main.c'


//Local function prototypes.
static char *merge_absolute (const char *argpath, const int *reserve);
static bool within_rootdir (char *fullpath, const char *trimmed);
static bool is_a_dir (const char *fullpath);
static int is_unique (const char *fullpath);
static char *trim_arg_path (char **argpath, int *reserve);
static void restore_trimmed (char **argpath, char **fullpath, char *trimmed);


/******************************************************************************
 * check_file_exist - see path.h
 *****************************************************************************/
bool check_file_exist (const char *cwd, char *argpath)
{
  char *fullpath;

  //Merge all parts of the pathname to create a single pathname.
  if ((fullpath = merge_paths (cwd, argpath, NULL)) == NULL)
    return false;

  //Ensure the pathname is a descendant of the servers root directory.
  if (!within_rootdir (fullpath, NULL)) {
    free (fullpath);
    return false;
  }

  free (fullpath);
  return true;
}


/******************************************************************************
 * check_dir_exist - see path.h
 *****************************************************************************/
bool check_dir_exist (const char *cwd, char *argpath)
{
  char *fullpath;

  //Merge all parts of the pathname to create a single pathname.
  if ((fullpath = merge_paths (cwd, argpath, NULL)) == NULL)
    return false;

  //Determine if the pathname is a descendant of the servers root directory.
  if (!within_rootdir (fullpath, NULL)) {
    free (fullpath);
    return false;
  }

  //Determine if the pathname is for a directory.
  if (!is_a_dir (fullpath)) {
    free (fullpath);
    return false;
  }

  free (fullpath);
  return true;
}


/******************************************************************************
 * check_future_file - see path.h
 *****************************************************************************/
int check_future_file (const char *cwd, char *argpath, bool unique)
{
  char *fullpath;
  char *trimmed;
  int reserve;
  int uniq_rv;

  if ((trimmed = trim_arg_path (&argpath, &reserve)) == NULL)
    return -1;

  if ((fullpath = merge_paths (cwd, argpath, &reserve)) == NULL)
    return -1;

  if (!within_rootdir (fullpath, trimmed)) {
    free (fullpath);
    free (trimmed);
    return -2;
  }

  //Trimmed is freed in this function call.
  restore_trimmed (&argpath, &fullpath, trimmed);

  /* Check if the file is unique when requested in the third argument to this
   * function. */
  if (unique) {
    if ((uniq_rv = is_unique (fullpath)) == 0) {
      free (fullpath);
      return 0;
    } else if (uniq_rv == -1) {
      free (fullpath);
      return -1;
    } else if (uniq_rv == -2) {
      free (fullpath);
      return -3;
    }
  }
    
  /* If the pathname argument received from the client is the pathname of a
   * directory, the new file cannot be created. */
  if (is_a_dir (fullpath)) {
    free (fullpath);
    return -3;
  }

  free (fullpath);
  return 0;
}


/******************************************************************************
 * merge_paths - see path.h
 *****************************************************************************/
char *merge_paths (const char *cwd, char *argpath, const int *reserve)
{
 //Concatenate the rootdir, cwd, and path argument relevant to the cwd.
  char *fullpath;
  char *start;

  //String lengths required to malloc the fullpath string.
  int rootdir_strlen;
  int cwd_strlen;
  int arg_strlen;
  int i = 0;


  /* This block of code removes ' ' (space) characters from a path argument.
   * These spaces appear in arguments sent by a web browser. Due to time
   * constraints, we have not been able to determine if this is expected
   * behaviour. This solution will remove the ability to work with files 
   * (including directories) that have space characters in their filename. If
   * we had more development time this would not be the case. */
  if (argpath != NULL) {
    start = argpath;
    char arg_temp[strlen (argpath) + 1];
    while (*argpath != '\0') {
      if (*argpath != ' ')
	arg_temp[i++] = *argpath++;
      else
	argpath++;
    }
    arg_temp[i] = '\0';
    argpath = strcpy (start, arg_temp);
  }


  //Merge the absolute path. Proceed if the path is not absolute.
  if ((fullpath = merge_absolute (argpath, reserve)) != NULL)
    return fullpath;

  /* When the argument begins with a '/' character, the path is absolute.
   * merge_absolute() will only return NULL for an absolute path when malloc()
   * has returned error. */
  if (fullpath == NULL) {
    if ((argpath != NULL) && (argpath[0] == '/'))
      return NULL;
  }


  rootdir_strlen = strlen (rootdir) + 1;
  cwd_strlen = strlen (cwd) + 1;

  //Additional string length calculations for a "trimmed" pathname.
  if (argpath == NULL) {
    if (reserve == NULL) { //Always malloc() the space requested in reserve.
      arg_strlen = 0;
    } else {
      arg_strlen = 0 + *reserve;
    }
  } else {
    arg_strlen = strlen (argpath) + 1;
  }

  if (reserve != NULL)
    arg_strlen += *reserve;

  if ((fullpath = malloc ((rootdir_strlen + cwd_strlen + arg_strlen)
			  * sizeof (*fullpath))) == NULL) {
    fprintf (stderr, "%s: malloc: could not allocate required space\n",
	     __FUNCTION__);
    return NULL;
  }

  //Create the complete pathname argument. 
  strcpy (fullpath, rootdir);
  strcat (fullpath, cwd);
  if (argpath != NULL)
    strcat (fullpath, argpath);

  return fullpath;
}


/******************************************************************************
 * This function is called by the function merge_paths(). When a path argument
 * begins with the character '/', the argument is an absolute path.
 *
 * When the path is absolute, the current working directory should not be
 * inserted between the path argument and rootdir path.
 *
 * If an error occurs in this function due to a malloc error, it is not caught
 * in this function. merge_paths(), the caller of this function, will detect
 * the malloc() error after calling this function.
 *
 * Arguments:
 *    argpath - A pathname argument for a client command.
 *    reserve - Add this count to the size of malloc(). View the function
 *              header for merge_paths() in "path.h" for a description.
 *
 * Return values:
 *   NULL - The path argument is not absolute, or malloc() error.
 *   string - The absolute path to argument.
 *****************************************************************************/
static char *merge_absolute (const char *argpath, const int *reserve)
{
  char *fullpath;     //Concatenate the rootdir and the absolute path argument.
  int rootdir_strlen;
  int argpath_strlen;
  int path_sz;

  //Do not attempt to merge rootdir with NULL, the merge is rootdir.
  if (argpath == NULL)
    return NULL;


  //When the path does not begin with '/', it is not absolute.
  if (argpath[0] != '/')
    return NULL;


  rootdir_strlen = strlen (rootdir);
  argpath_strlen = strlen (argpath) + 1;

  /* If the argument has been trimmed, reserve space for it to be restored. See
   * the function header for trim_arg_path() in this file. */
  if (reserve != NULL)
    argpath_strlen += *reserve;

  path_sz = argpath_strlen + rootdir_strlen + 1; //+1 for null terminator.


  if ((fullpath = malloc (path_sz * sizeof (*fullpath))) == NULL) {
    fprintf (stderr, "%s: malloc: could not allocate the required space\n",
	     __FUNCTION__);
    return NULL;
  }

  *fullpath = '\0';
  //Concatenate the absolute path to the argument.
  strcat (fullpath, rootdir);
  strcat (fullpath, argpath);

  return fullpath;
}

/******************************************************************************
 * Determine if a pathname includes the root directory of the server, as
 * chosen in the file 'ftp.conf'. This function is used when determining if a
 * pathname argument that was sent with a client command should be accepted.
 *
 * The pathname provided in the first argument will be canonicalized before
 * this comparison is made.
 *
 * Arguments:
 *   fullpath - The full pathname to the argument. May include symbolic links,
 *              "..", etc.
 *
 *   trimmed  - The full pathname provided in the first argument may have been
 *              shortened by a previous caller to this function. There is a
 *              very specific case where this argument is required, described
 *              below.
 *
 * Return values:
 *   true - The pathname is a descendant of the root directory set in the file
 *          'ftp.conf'.
 *   false - The pathname is not a descendant of the root directory.
 *
 * Original author: Evan Myers
 *
 * Regarding the trimmed argument:
 *   When canonicalize_file_name() is called, if any component of a pathname
 *   is not a file, it will return NULL and set errno appropriately. A function
 *   that calls this function may be trying to determine if a file may be
 *   created with the supplied pathname argument.
 *
 *   To allow this function to work correctly, the caller function has trimmed
 *   the file which may not exist. This trimming process removes the file from
 *   all prefix components (the path).
 *
 *   If the prefix component ends with the entry "..", and the filename is the
 *   current directory, an error will be reported where it should not be.
 *
 *   This error needs to be reported correctly, so that the server may send the
 *   proper reply to the client.
 *****************************************************************************/
static bool within_rootdir (char *fullpath, const char *trimmed)
{
  char *canon;       //An abbreviation of canonicalized absolute pathname.
  char *str;         //Return value of strcat function.

  //Resolve all "..", ".", and duplicate '/' entries. Resolve symbolic links.
  if ((canon = canonicalize_file_name (fullpath)) == NULL) {
    fprintf (stderr, "%s: canonicalize_file_name: %s\n", __FUNCTION__, 
	     strerror (errno));
    return false;
  }

  //Determine if the pathname is a descendant of the servers root directory.
  if (strstr (canon, rootdir) == NULL) {
    /* This if statement handles a very special case. See the notes in the
     * function header. Restore the trimmed filename and call this function
     * again. NULL must be passed as the second loop to prevent an infinite
     * loop. */
    if (trimmed != NULL) {
      str = strcat (fullpath, trimmed);
      if (!within_rootdir (fullpath, NULL)) {
	*str = '\0';
	free (canon);
	return false;
      }
      *str = '\0';
    } else {
      fprintf (stderr, "%s: pathname out of rootdir scope\n", __FUNCTION__);
      free (canon);
      return false;
    }
  }

  free (canon);
  return true;
}


/******************************************************************************
 * Determines if a file is a directory.
 *
 * Arguments:
 *   fullpath - The complete path to a file.
 *
 * Return values:
 *   true - The file is a directory.
 *   bool - The file is not a directory.
 *
 * Original author: Evan Myers
 *****************************************************************************/
bool is_a_dir (const char *fullpath)
{
  struct stat st;  //Used to check if the file at pathname is a directory.

  //stat() the file to determine if the file is a directory.
  if (stat (fullpath, &st) == -1) {
    fprintf (stderr, "%s: stat: %s\n", __FUNCTION__, strerror (errno));
    return false;
  }

  //Determine if the file is a directory. See man (2) stat, line 120.
  if (!(st.st_mode & S_IFDIR)) {
    return false;
  }
  return true;
}


/******************************************************************************
 * Separate a filename from the file path.
 *
 * Arguments:
 *  argpath - The filename argument received with a client command. Remove the
 *            filename from all prefix components of the path.
 *  reserve - Set the number of characters removed from the first argument in
 *            this argument on function return.
 *
 * Return values:
 *  NULL - error, or no filename was trimmed.
 *
 * Original author: Evan Myers
 *****************************************************************************/
static char *trim_arg_path (char **argpath, int *reserve)
{
  char *trim_filen = NULL;  //Store the trimmed portion of the filename.
  char *str;                //Used to collect the filename from the prefix path.

  //Determine if the argument contains a pathname prefix.
  if ((str = strrchr(*argpath, '/')) != NULL) {
    if (strlen (*argpath) > 1) //required when the pathname argument is "/".
      str++;              //Do not include the '/' prefix in the new filename.
  } else {
    //The filename does not include a path prefix (eg. no '/' is present).
    str = *argpath;
  }

  /* Do not remove any trailing ".." entries. This function is always called
   * before resolving the pathname. */
  if (strcmp (str, "..") == 0)
    str = NULL;

  //Nothing has been trimmed. Set return values and exit the function.
  if (str == NULL) {
    *reserve = 0;
    return NULL;
  }
  
  //Set the amount to be trimmed.
  *reserve = strlen (*argpath) + 1;

  /* Collect the filename from the prefix path.
   * eg. collect "filename" from "prefix/filename". */
  if ((trim_filen = malloc (*reserve * sizeof (*trim_filen))) == NULL) {
    fprintf (stderr, "%s: malloc: could not allocate required space\n",
	     __FUNCTION__);
    return NULL;
  }

  /* Copy the string and remove the filename from the prefix path. This
   * operation will replace a trailing filename with a null character so that
   * future string functions only recognize the prefix.
   * eg. "prefix/trail" becomes "prefix/" */
  strncpy (trim_filen, str, *reserve);
  *str = '\0';
  
  return trim_filen;
}


/******************************************************************************
 * Restore all strings effected by the trimming process to the original state.
 *
 * Arguments:
 *   argpath - Set to the original state before the trimming on function return.
 *  fullpath - Set to the original state before the trimming on function return.
 *  trimmed  - The filename that has been trimmed from the strings. This
 *             argument will be freed after the other arguments have been
 *             restored.
 * 
 * Original author: Evan Myers
 *****************************************************************************/
static void restore_trimmed (char **argpath, char **fullpath, char *trimmed)
{
  strcat (*argpath, trimmed);
  strcat (*fullpath, trimmed);
  free (trimmed);
}


/******************************************************************************
 * Some functions may require the pathname argument received with a client
 * command be accepted only when no other file exists with that name. This
 * function will do that.
 *
 * Arguments:
 *  fullpath - Set to the original state before the trimming on function return.
 *
 * Return values:
 *   0    The file does not exist
 *  -1    There was an error (NOT errno ENOENT) with the call to stat().
 *  -2    The file exists, a file created with fullpath would not be unique.
 *
 * Original author: Evan Myers
 *
 * Note: Special care must be taken when modifying this function. The function
 *       only returns 0 (a positive response) when the call to stat() fails
 *       because the file does not exist.
 *****************************************************************************/
static int is_unique (const char *fullpath)
{
  struct stat st;  //Used to check if the file at pathname is a directory.

  if (stat (fullpath, &st) == -1) {
    if (errno == ENOENT) { //Return true if the file does not exist.
      return 0;
    } else {
      fprintf (stderr, "%s: stat: %s\n", __FUNCTION__, strerror (errno));
      return -1;
    }
  } else {
    fprintf (stderr, "%s: stat: FUTURE_UNIQ file exists\n", __FUNCTION__);
    return -2;
  }
}
