/******************************************************************************
 * Students: Evan Myers, Justin Slind, Alex Tai, James Yoo
 * Course: CMPT-361
 * Assignment #3 - ftp server
 * Files: directory.c
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


#define MAX_FDATSZ 4096


//Local function prototypes.
static void listDirect (session_info_t *si, char * fullpath, bool detail);
static int detailList (struct dirent *dirInfo, char *fullpath, char **output);


extern char *rootdir; //The root directory of the server, defined in 'main.c'.


//TODO: What does this do? Why is this here?
char fileBuff[MAX_FDATSZ];


/******************************************************************************
 * cmd_list_nlst - see "directory.h"
 *****************************************************************************/
void cmd_list_nlst (session_info_t *si, char *arg, bool detail)
{
  char *fullpath;
  char *noAccess;
  char *transferStart;
  char *noConnection;
  

 if (si->logged_in == false) {
    noAccess = "550 - Access denied.\n";
    send_all(si->c_sfd, (uint8_t *)noAccess, strlen(noAccess));
    return;
  }

  //Determine if the file is a directory.
  if (!check_dir_exist (si->cwd, arg)) {
    send_mesg_553 (si->c_sfd);
    return;
  }

  transferStart = "150 Here comes the directory listing.\n";
  send_all(si->c_sfd,(uint8_t*)transferStart,strlen(transferStart));

  if (si->d_sfd == 0) {
    noConnection = "425 - Cannot open data connection; please use the PORT or PASV command first.\n";
    send_all(si->c_sfd, (uint8_t *)noConnection, strlen(noConnection));
    return;
  }

  //Create a single pathname to the directory from the pathname fragments.
  if ((fullpath = merge_paths (si->cwd, arg, NULL)) == NULL) {
    send_mesg_451 (si->c_sfd);
    close (si->d_sfd);
    si->d_sfd = 0;
    return;
  }

  listDirect (si, fullpath, detail);
  free (fullpath);
  return;
}


/******************************************************************************
 * List the contents of a directory and send the results to the client on the
 * data connection.
 *
 * This function will call the detailList() static helper function to modify
 * the output to include more information with each filename.
 *
 * Arguments:
 *        si - A pointer to the session information maintained by session().
 *   argpath - The filename argument received with the list or nlist command.
 *    detail - Set to true if a detailed listing was requested.
 *
 * Original Author: Alex Tai
 * Rewritten by: Evan Myers
 *****************************************************************************/
static void listDirect (session_info_t *si, char * fullpath, bool detail) {
  DIR *dp;                       //directory pointer
  struct dirent *ep;             //entry pointer
  char *output;                  //output buffer
  int outSize = CMD_STRLEN;
  char *aborted;
  char *success;

  //Open the directory to be listed.
  if ((dp = opendir(fullpath)) == NULL) {
    fprintf (stderr, "%s: opendir: %s\n", __FUNCTION__, strerror (errno));
    send_mesg_451 (si->c_sfd);
    close (si->d_sfd);
    si->d_sfd = 0;
    return;
  }

  //Create an output buffer.
  output = calloc(outSize, sizeof(*output));
  if(output == NULL){
    fprintf(stderr, "Error in allocating memory for output in list.");
    send_mesg_451 (si->c_sfd);
    close (si->d_sfd);
    si->d_sfd = 0;
    return;
  }

  //Read all entries from the directory.
  errno = 0;
  while(((ep = readdir(dp)) != NULL) && (si->cmd_abort == false)){
    //Do not list hidden files, current directory, or parent directory.
    if(ep->d_name[0] != '.'){
      //Create a detailed long listing, or a simple filename listing.
      if(detail == true){
	detailList(ep, fullpath, &output);
      } else {
	strcat(output, ep->d_name);
	strcat(output, "\r\n");
      }

      //Expand the buffer when appropriate.
      if(strlen(output) >= (outSize-356)){
      	outSize += CMD_STRLEN;
      	if ((output = realloc(output, outSize * sizeof (*output))) == NULL) {
	  fprintf (stderr, "%s: realloc: %s\n", __FUNCTION__,
		   "could not allocate the required space");
	  send_mesg_451 (si->c_sfd);
	  close (si->d_sfd);
	  si->d_sfd = 0;
	  return;
	}
      }
    }
  }

  //checks to see if its and error or eof
  if(ep == NULL){
    if(errno){
      fprintf(stderr, "%s: readdir: %s\n", __FUNCTION__, strerror(errno));
      send_mesg_451 (si->c_sfd);
      close (si->d_sfd);
      si->d_sfd = 0;
      closedir (dp);
      return;
    }
  }

  //Send the directory listing.
  send_all (si->d_sfd, (uint8_t*)output, strlen (output));
  free (output);

  //Send the appropriate message if the command was aborted.
  if (si->cmd_abort == true) {
    aborted = "426 - Connection close; transfer aborted.\n";
    send_all(si->c_sfd, (uint8_t *)aborted, strlen(aborted));
    si->cmd_abort = false;
  } else {
    success = "226 - Closing data connection; requested file action successful.\n";
    send_all(si->c_sfd, (uint8_t *)success, strlen(success));
  }

  //Clean up before returning.
  if (closedir (dp) == -1)
    fprintf (stderr, "%s: closedir: %s\n", __FUNCTION__, strerror (errno));
  close (si->d_sfd);
  si->d_sfd = 0;
  return;
}


/******************************************************************************
 * Collect the detailed file information of a directory entry (a file or a
 * directory present in a directory), and append this formatted information
 * to the output string used by the function listDirect().
 *
 * Arguments:
 *   dirInfo - The name of a file present in the directory listing.
 *  fullpath - The canonicalized filename to the directory entry.
 *    output - Append the detailed file information to this string.
 *
 * Return values:
 *   0  Success
 *  -1  Error
 *
 * Original Author: Alex Tai
 * Rewritten by: Evan Myers
 *****************************************************************************/
static int detailList (struct dirent *dirInfo, char *fullpath, char **output) {
  //cmtime() requires 26 characters. We will use less characters in our string.
  char time[26];  
  struct stat fileStat;
  struct tm * timeinfo;
  int errchk;
  errno = 0;

  //Add the size of d_name field in struct stat, + 1 for a null terminator.
  char filename[strlen (fullpath) + 256];
  filename[0] = '\0';
  strcat (filename, fullpath);
  strcat (filename, dirInfo->d_name);

  errchk = stat (filename, &fileStat);

  // Check to see if stat() encountered any error
  if(errchk == -1){
    fprintf(stderr, "Error using stat function: %s\n", strerror(errno));
    return -1;
  }

  //Prepare the time returned from stat for a call to strftime().
  if ((timeinfo = gmtime (&fileStat.st_mtime)) == NULL) {
    fprintf (stderr, "%s: gmtime: error with function call\n", __FUNCTION__);
    return -1;
  }
  strftime (time, 20, "%b %d %Y", timeinfo);

  //Store the type of file to the output string.
  if(dirInfo->d_type == DT_DIR){
    strcat(*output, "d");
  } else if (dirInfo->d_type == DT_LNK) {
    strcat(*output, "l");
  } else {
    strcat(*output, "-");
  }

  //Store the permissions to the output string.
  (fileStat.st_mode & S_IRUSR) ? strcat(*output,"r"):strcat(*output,"-");
  (fileStat.st_mode & S_IWUSR) ? strcat(*output,"w"):strcat(*output,"-");
  (fileStat.st_mode & S_IXUSR) ? strcat(*output,"x"):strcat(*output,"-");
  (fileStat.st_mode & S_IRGRP) ? strcat(*output,"r"):strcat(*output,"-");
  (fileStat.st_mode & S_IWGRP) ? strcat(*output,"w"):strcat(*output,"-");
  (fileStat.st_mode & S_IXGRP) ? strcat(*output,"x"):strcat(*output,"-");
  (fileStat.st_mode & S_IROTH) ? strcat(*output,"r"):strcat(*output,"-");
  (fileStat.st_mode & S_IWOTH) ? strcat(*output,"w"):strcat(*output,"-");
  (fileStat.st_mode & S_IXOTH) ? strcat(*output,"x"):strcat(*output,"-");

  //Store the other listing requirements to the output string.
  sprintf ((*output) + strlen(*output), "%zu\t%d\t%d\t%lld\t%s\t%s\r\n",
	   fileStat.st_nlink, fileStat.st_uid, fileStat.st_gid,
	   (unsigned long long)fileStat.st_size, time, dirInfo->d_name);
  
  return 0;
}


/******************************************************************************
 * makeDir - see "directory.h"
 *****************************************************************************/
void makeDir (session_info_t *si, char * filepath) {
  mode_t permissions = 0;
  //Sets permission for the new folder being created.
  permissions = permissions | S_IRUSR;
  permissions = permissions | S_IWUSR;
  permissions = permissions | S_IXUSR;

  // Checks to make sure that only a logged in user can make a
  // new directory as well as the user is NOT anonymous.
  if (si->logged_in == false || strcmp (si->user, "anonymous") == 0) {
    char *response = "550 Please login with USER and PASS.\n";
    send_all (si->c_sfd, (uint8_t *)response, strlen (response));
    close (si->d_sfd);
    si->d_sfd = 0;
    return;
  }

  if ((filepath = merge_paths (si->cwd, filepath, NULL)) == NULL) {
    fprintf (stderr, "%s: mkdir: filepath merge error\n", __FUNCTION__);
    close (si->d_sfd);
    si->d_sfd = 0;
    return;
  }

  if (mkdir (filepath, permissions) == -1) {
    fprintf (stderr, "%s: mkdir: %s\n", __FUNCTION__, strerror (errno));
    send_mesg_550 (si->c_sfd);
    close (si->d_sfd);
    si->d_sfd = 0;
    free (filepath);
    return;
  }

  /* trims and modified the displayed filepath of the new directory so that it
   * will not display the path above the specified root directory and root
   * directory is displayed as "/" */
  char * printStart = "257 - ";
  char * printEnd = "/ \n";
  char * root = rootdir;
  char * outpath = strstr(filepath, root);
  outpath += strlen(root);
  send_all (si->c_sfd, (uint8_t *)printStart, strlen (printStart));
  send_all (si->c_sfd, (uint8_t *)outpath, strlen (outpath));
  send_all (si->c_sfd, (uint8_t *)printEnd, strlen (printEnd));

  free (filepath);
  return;
}


/******************************************************************************
 * cmd_cdup - see "directory.h"
 *****************************************************************************/
void cmd_cdup (session_info_t *si, char *arg)
{
  char *response;

  if (si->logged_in == false) {
    response = "550 - Must login with USER and PASS.\n";
    send_all(si->c_sfd, (uint8_t *)response, strlen (response));
    return;
  }

  if (arg != NULL) {
    response = "550 No argument allowed.\n";
    send_all (si->c_sfd, (uint8_t *)response, strlen (response));
    return;
  }

  //Call cmd_cwd() to change to the parent directory.
  char up[10];
  strcpy (up, "..");
  cmd_cwd (si, up);
  return;
}


/******************************************************************************
 * cmd_cwd - see "directory.h"
 *****************************************************************************/
void cmd_cwd (session_info_t *si, char *arg)
{
  char *response;
  char *fullpath;
  char *canon;

  if (si->logged_in == false) {
    response = "550 Please login with USER and PASS.\n";
    send_all (si->c_sfd, (uint8_t *)response, strlen (response));
    return;
  }
  
  /* Determine if the file is a directory and within the server root directory
   * which can be found in the file 'ftp.conf'. */
  if (!check_dir_exist (si->cwd, arg)) {
    response = "550 Directory not found.\n";
    send_all (si->c_sfd, (uint8_t *)response, strlen (response));
    return;
  }

  //Create a single pathname to the directory from the pathname fragments.
  if ((fullpath = merge_paths (si->cwd, arg, NULL)) == NULL) {
    response = "550 Error while processing.\n";
    send_all (si->c_sfd, (uint8_t *)response, strlen (response));
    return;
  }

  /* Canonicalize the file name so that a longer session will not contain
   * multiple ".." entries in the pathname, making it difficult for the client
   * to determine where they are. */
  if ((canon = canonicalize_file_name (fullpath)) == NULL) {
    response = "550 Error while processing.\n";
    send_all (si->c_sfd, (uint8_t *)response, strlen (response));
    free (fullpath);
    return;
  }

  si->cwd[0] = '\0';
  /* Copy the path found after the rootdir to the session cwd. Only the cwd is
   * modified in this process. */
  strcat (si->cwd, canon + strlen (rootdir));
  /* The implementation of our paths require cwd to always be followed by a
   * directory separator. (rootdir  --->   / <<cwd> />  ----> argument) */
  strcat (si->cwd, "/");

  response = "250 Working directory changed.\n";
  send_all (si->c_sfd, (uint8_t *)response, strlen (response));
  
  free (fullpath);
  free (canon);
  return;
}
