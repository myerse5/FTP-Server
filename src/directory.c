/******************************************************************************
 * FTP-Server
 * Authors: Evan Myers, Justin Slind, Alex Tai, James Yoo
 * Date: November 2013
 * 
 * Description:
 *   FTP functions that create or change directories.
 *****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <inttypes.h>
#include <time.h>
#include "directory.h"
#include "net.h"
#include "path.h"
#include "reply.h"
#include "session.h"


#define MAX_FDATSZ 4096  // TODO integrate into standard buffer size with an
                         // actual error check involved.


// Local function prototypes.
static void list_directory (session_info_t *si, char * fullpath, bool detail);
static int detail_list (struct dirent *dirInfo, char *fullpath, char **output);


extern char *rootdir; // The root directory of the server, defined in 'main.c'.


// TODO: integrate into standard buffer size with an actual error check involved
char fileBuff[MAX_FDATSZ];


/******************************************************************************
 * cmd_list_nlst - see "directory.h"
 *****************************************************************************/
void cmd_list_nlst (session_info_t *si, char *arg, bool detail)
{
  char *fullpath;
  int dsfd;
  int csfd = si->csfd;

  char *hostname = si->connInfo.hostname;
  char *port = si->connInfo.port;
  int *pasv = &(si->connInfo.pasv);
  
  if (!si->loggedin) {
    send_mesg_530 (csfd, REPLY_530_REQUEST);
    return;
  }
  
  // Determine if the file is a directory.
  if (!check_dir_exist (si->cwd, arg)) {
    send_mesg_550_no_dir (csfd);
    return;
  }

  printf ("host = %s\nport = %s\n", hostname, port);
  send_mesg_150 (csfd, NULL, REPLY_150_DIR);

  // Establish the data connection.
  if (si->connInfo.pasv > 0) {
    if ((dsfd = accept_connection (*pasv, ACCEPT_PASV, si)) == -1) {
      if (close (*pasv) == -1)
	fprintf (stderr, "%s: close: %s\n", __FUNCTION__, strerror (errno));
      *pasv = 0;
      send_mesg_425 (csfd);
      return;
    }
  } else if (strlen (si->connInfo.hostname) > 0) {
    if ((dsfd = port_connect (hostname, port)) == -1) {
      *hostname = '\0';
      *port = '\0';
      send_mesg_425 (csfd);
      return;
    }
  } else {
    send_mesg_425 (csfd);
    return;
  }
  si->dsfd = dsfd;

  // Create a single pathname to the directory from the pathname fragments.
  if ((fullpath = merge_paths (si->cwd, arg, NULL)) == NULL) {
    send_mesg_451 (csfd);
    close (si->dsfd);
    si->dsfd = 0;
    return;
  }

  list_directory (si, fullpath, detail);
  send_mesg_226 (csfd, REPLY_226_SUCCESS);
  
  free (fullpath);
  
  *hostname = '\0';
  *port = '\0';
  *pasv = 0;
  close (si->dsfd);
  si->dsfd = 0;

  printf ("return from nlst/list\n");
  return;
}


/******************************************************************************
 * List the contents of a directory and send the results to the client on the
 * data connection.
 *
 * This function will call the detail_list() static helper function to modify
 * the output to include more information with each filename.
 *
 * Arguments:
 *        si - A pointer to the session information maintained by session().
 *   argpath - The filename argument received with the list or nlist command.
 *    detail - Set to true if a detailed listing was requested.
 *****************************************************************************/
static void list_directory (session_info_t *si, char * fullpath, bool detail)
{
  DIR *dp;                       // directory pointer
  struct dirent *ep;             // entry pointer
  char *output;                  // output buffer
  int outSize = CMD_STRLEN;
  int csfd;                      // Control socket file descriptor.

  csfd = si->csfd;

  // Open the directory to be listed.
  if ((dp = opendir (fullpath)) == NULL) {
    fprintf (stderr, "%s: opendir: %s\n", __FUNCTION__, strerror (errno));
    send_mesg_451 (csfd);
    close (si->dsfd);
    si->dsfd = 0;
    return;
  }

  // Create an output buffer.
  output = calloc (outSize, sizeof(*output));
  if(output == NULL){
    fprintf (stderr, "%s: calloc of %d bytes failed\n", __FUNCTION__, outSize);
    send_mesg_451 (csfd);
    close (si->dsfd);
    si->dsfd = 0;
    return;
  }

  errno = 0;
  // Read all entries from the directory.
  while ((ep = readdir (dp)) != NULL && (si->cmdAbort == false)) {
    // Do not list hidden files, current directory, or parent directory.
    if (ep->d_name[0] != '.') {
      // Create a detailed long listing, or a simple filename listing.
      if (detail == true) {
	detail_list (ep, fullpath, &output);
      } else {
	strcat (output, ep->d_name);
	strcat (output, "\r\n");
      }

      // Expand the buffer when appropriate.
      if (strlen (output) >= (outSize-356)) {
      	outSize += CMD_STRLEN;
      	if ((output = realloc (output, outSize * sizeof(*output))) == NULL) {
	  fprintf (stderr, "%s: realloc of %d bytes failed\n",
		   __FUNCTION__, outSize);
	  send_mesg_451 (csfd);
	  close (si->dsfd);
	  si->dsfd = 0;
	  return;
	}
      }
    }
  }

  /* Send an error message and exit the if readdir() failed, proceed when ep
   * returned NULL due to EOF. */
  if (ep == NULL) {
    if (errno) {
      fprintf (stderr, "%s: readdir: %s\n", __FUNCTION__, strerror (errno));
      send_mesg_451 (csfd);
      close (si->dsfd);
      si->dsfd = 0;
      closedir (dp);
      return;
    }
  }

  // Send the directory listing.
  send_all (si->dsfd, (uint8_t*)output, strlen (output));
  free (output);

  // Send the appropriate message if the command was aborted.
  if (si->cmdAbort == true) {
    send_mesg_426 (csfd);
    si->cmdAbort = false;
  }

  // Clean up before returning.
  if (closedir (dp) == -1)
    fprintf (stderr, "%s: closedir: %s\n", __FUNCTION__, strerror (errno));

  return;
}


/******************************************************************************
 * Collect the detailed file information of a directory entry (a file or a
 * directory present in a directory), and append this formatted information
 * to the output string used by the function list_directory().
 *
 * Arguments:
 *   dirInfo - The name of a file present in the directory listing.
 *  fullpath - The canonicalized filename to the directory entry.
 *    output - Append the detailed file information to this string.
 *
 * Return values:
 *   0  Success
 *  -1  Error
 *****************************************************************************/
static int detail_list (struct dirent *dirInfo, char *fullpath, char **output)
{
  // cmtime() requires 26 characters. We will use less characters in our string.
  char time[26];  // Change this to a constant TODO.
  struct stat fileStat;
  struct tm * timeinfo;
  errno = 0;

  /* Add the size of d_name field in struct stat (255), + 1 for a null
   * terminator, to the fullpath of the cwd. */
  char filename[strlen (fullpath) + 256];
  filename[0] = '\0';
  strcat (filename, fullpath);
  strcat (filename, dirInfo->d_name);

  //  Check to see if stat() encountered any error
  if (stat (filename, &fileStat) == -1) {
    fprintf (stderr, "%s: stat: %s\n", __FUNCTION__, strerror (errno));
    return -1;
  }

  // Prepare the time returned from stat for a call to strftime().
  if ((timeinfo = gmtime (&fileStat.st_mtime)) == NULL) {
    fprintf (stderr, "%s: gmtime: failed\n", __FUNCTION__);
    return -1;
  }
  strftime (time, 20, "%b %d %Y", timeinfo);

  // Store the type of file to the output string.
  if(dirInfo->d_type == DT_DIR){
    strcat(*output, "d");
  } else if (dirInfo->d_type == DT_LNK) {
    strcat(*output, "l");
  } else {
    strcat(*output, "-");
  }

  // Store the permissions to the output string.
  (fileStat.st_mode & S_IRUSR) ? strcat(*output,"r"):strcat(*output,"-");
  (fileStat.st_mode & S_IWUSR) ? strcat(*output,"w"):strcat(*output,"-");
  (fileStat.st_mode & S_IXUSR) ? strcat(*output,"x"):strcat(*output,"-");
  (fileStat.st_mode & S_IRGRP) ? strcat(*output,"r"):strcat(*output,"-");
  (fileStat.st_mode & S_IWGRP) ? strcat(*output,"w"):strcat(*output,"-");
  (fileStat.st_mode & S_IXGRP) ? strcat(*output,"x"):strcat(*output,"-");
  (fileStat.st_mode & S_IROTH) ? strcat(*output,"r"):strcat(*output,"-");
  (fileStat.st_mode & S_IWOTH) ? strcat(*output,"w"):strcat(*output,"-");
  (fileStat.st_mode & S_IXOTH) ? strcat(*output,"x"):strcat(*output,"-");

  // Store the other listing requirements to the output string.
  sprintf ((*output) + strlen(*output), "%zu\t%d\t%d\t%lld\t%s\t%s\r\n",
	   fileStat.st_nlink, fileStat.st_uid, fileStat.st_gid,
	   (unsigned long long)fileStat.st_size, time, dirInfo->d_name);
  
  return 0;
}


/******************************************************************************
 * cmd_mkd - see "directory.h"
 *****************************************************************************/
void cmd_mkd (session_info_t *si, char * filepath)
{
  mode_t permissions;
  char *outpath;
  int csfd = si->csfd;       // Control socket file descriptor.

  // Sets permission for the new folder being created.
  permissions = 0;
  permissions = permissions | S_IRUSR;
  permissions = permissions | S_IWUSR;
  permissions = permissions | S_IXUSR;

  //  Checks to make sure that only a logged in user can make a
  //  new directory as well as the user is NOT anonymous.
  if (si->loggedin == false || strcmp (si->user, "anonymous") == 0) {
    send_mesg_530 (csfd, REPLY_530_REQUEST);
    close (si->dsfd);
    si->dsfd = 0;
    return;
  }

  if ((filepath = merge_paths (si->cwd, filepath, NULL)) == NULL) {
    fprintf (stderr, "%s: merge_paths: filepath merge error\n", __FUNCTION__);
    close (si->dsfd);
    si->dsfd = 0;
    return;
  }

  if (mkdir (filepath, permissions) == -1) {
    fprintf (stderr, "%s: mkdir: %s\n", __FUNCTION__, strerror (errno));
    send_mesg_550_process_error (csfd);
    close (si->dsfd);
    si->dsfd = 0;
    free (filepath);
    return;
  }

  /* Trims and modified the displayed filepath of the new directory so that it
   * will not display the path above the specified root directory and root
   * directory is displayed as "/". */
  outpath = strstr (filepath, rootdir);
  outpath += strlen (rootdir);
  send_mesg_257 (csfd, outpath);

  /* TESTING send_mesg_257, block commented out before being deleted.
   * char *printStart = "257 - ";
   * char *printEnd = "/ \n";
   * char *root = rootdir;
   * char *outpath = strstr (filepath, root);
   * outpath += strlen (root);
   * send_all (si->csfd, (uint8_t *)printStart, strlen (printStart));
   * send_all (si->csfd, (uint8_t *)outpath, strlen (outpath));
   *send_all (si->csfd, (uint8_t *)printEnd, strlen (printEnd));
   */

  free (filepath);
  return;
}


/******************************************************************************
 * cmd_cdup - see "directory.h"
 *****************************************************************************/
void cmd_cdup (session_info_t *si, char *arg)
{
  char up[10];
  int csfd = si->csfd;

  if (si->loggedin == false) {
    send_mesg_530 (csfd, REPLY_530_REQUEST);
    return;
  }

  if (arg != NULL) {
    send_mesg_550_no_argument (csfd);
    return;
  }

  // Call cmd_cwd() to change to the parent directory.
  up[0] = '\0';
  strcpy (up, "..");
  cmd_cwd (si, up);
  return;
}


/******************************************************************************
 * cmd_cwd - see "directory.h"
 *****************************************************************************/
void cmd_cwd (session_info_t *si, char *arg)
{
  char *fullpath;
  char *canon;
  int csfd  = si->csfd;

  if (si->loggedin == false) {
    send_mesg_530 (csfd, REPLY_530_REQUEST);
    return;
  }
  
  /* Determine if the file is a directory and within the server root directory
   * which can be found in the file 'ftp.conf'. */
  if (!check_dir_exist (si->cwd, arg)) {
    send_mesg_550_no_dir (csfd);
    return;
  }

  // Create a single pathname to the directory from the pathname fragments.
  if ((fullpath = merge_paths (si->cwd, arg, NULL)) == NULL) {
    send_mesg_550_process_error (csfd);
    return;
  }

  /* Canonicalize the file name so that a longer session will not contain
   * multiple ".." entries in the pathname, making it difficult for the client
   * to determine where they are. */
  if ((canon = canonicalize_file_name (fullpath)) == NULL) {
    send_mesg_550_process_error (csfd);
    free (fullpath);
    return;
  }

  si->cwd[0] = '\0';
  /* Copy the path found after the rootdir to the session cwd. Only the cwd is
   * modified in this process. */
  strcat (si->cwd, canon + strlen (rootdir));
  /* The implementation of our paths require cwd to always be followed by a
   * directory separator. (rootdir="/" --->  <<cwd>/>  ----> argument) */
  strcat (si->cwd, "/");

  send_mesg_250 (csfd);
  
  free (fullpath);
  free (canon);
}


/******************************************************************************
 * cmd_pwd - see "directory.h"
 *****************************************************************************/
void cmd_pwd (session_info_t *si)
{
  send_mesg_257 (si->csfd, si->cwd);

  /* THIS IS COMMENTED OUT WHILE TESTING send_mesg_257 ()
   * char *printStart = "257 - \"";
   * char *printEnd = "\".\n";
   * 
   * send_all (si->csfd, (uint8_t *)printStart, strlen (printStart));
   * send_all (si->csfd, (uint8_t *)si->cwd, strlen (si->cwd));
   * send_all (si->csfd, (uint8_t *)printEnd, strlen (printEnd));
   */
}
