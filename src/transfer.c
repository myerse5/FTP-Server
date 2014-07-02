/******************************************************************************
 * FTP-Server
 * Authors: Evan Myers, Justin Slind, Alex Tai, James Yoo
 * Date: November 2013
 *
 * Description:
 *   FTP commands that transfer a file between the client and server. The STOR
 *   STOU, APPE, RETR commands are found in this file.
 *****************************************************************************/
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "transfer.h"
#include "net.h"
#include "path.h"
#include "reply.h"
#include "session.h"


//Local function prototypes.
static int perm_neg_check (session_info_t *si, char *arg);
static void store (session_info_t *si, char *cmd, char *purp);


//TODO Consolidate and check this (arbitrary?) value.
#define BUFFSIZE 1000


/******************************************************************************
 * cmd_stou - see "cmd_stor.h"
 *****************************************************************************/
void cmd_stou (session_info_t *si, char *arg)
{
  int rt;
  char *reply;
  char tempname[256];
  char *fullpath;

  //The user must be logged in on, and must not be anonymous.
  if (si->loggedin == false || strcmp (si->user,"anonymous") == 0) {
    reply = "550 Permission denied.\n";
    send_all (si->csfd, (uint8_t*)reply, strlen (reply));
    close (si->dsfd);
    si->dsfd = 0;
    return;
  }

  /* RFC 959 does not require or expect parameter for STOU cmd. Future updates
   * to the server will allow the user to recommend a filename. */
  srand (time (NULL));
  do {
    if (fullpath)
      free (fullpath);
    sprintf (tempname, "%d", rand());
    fullpath = merge_paths (si->cwd, tempname, NULL);
    /* F_OK tests if a file exists. The file must not exist (-1) to store the
     * file with a unique name. */
    if ((rt = access (fullpath, F_OK)) == -1) {
      if (errno == ENOENT)
	continue;
      else
	return;
    }
  } while (rt != -1);

  free (fullpath);
  store (si, tempname, "w");
  return;
}


/******************************************************************************
 * cmd_stor - see "transfer.h"
 *****************************************************************************/
void cmd_stor (session_info_t *si, char *cmd)
{
  if (perm_neg_check (si, cmd) == -1)
    return;
  
  store (si, cmd, "w");
  return;
}


/******************************************************************************
 * cmd_appe - see "transfer.h"
 *****************************************************************************/
void cmd_appe (session_info_t *si, char *cmd)
{
  if (perm_neg_check (si, cmd) == -1)
    return;
  
  store(si, cmd, "a");
  return;
}


/******************************************************************************
 * Stores or appends a file to the servers file system, which action is
 * performed is determined by the 'purp' argument.
 *
 * Note: The purp argument characters are equivalent to the second argument to
 *       fopen(). See the fopen() manpage for a full list of options. Read
 *       options should not be passed to this function.
 *
 * Arguments:
 *   cmd - current command with parameter
 *    si - info for current session
 *  purp - "purpose", a character to be used with fopen, write or append.
 *****************************************************************************/
static void store (session_info_t *si, char *cmd, char *purp)
{
  struct timeval timeout;
  fd_set rfds;
  int nfds;

  FILE *storfile;
  int rv;
  char buffer[BUFFSIZE];

  //Strings used to send reply messages.
  char *reply;
  char *type;

  //Used to create the absolute path on the file system.
  char *fullpath;
  
  //send positive preliminary reply
  reply = "150 Opening ";
  send_all (si->csfd, (uint8_t*)reply, strlen (reply));
 
  if (si->type == 'a')
    type = "ASCII";
  else
    type = "BINARY";
  send_all(si->csfd,(uint8_t*)type, strlen(type));

  reply = " mode data connection for ";
  send_all (si->csfd, (uint8_t*)reply, strlen (reply));

  send_all (si->csfd, (uint8_t*)cmd, strlen (cmd));

  reply = ".\n";
  send_all (si->csfd, (uint8_t*)reply, strlen (reply));
  
  /* Data connection must already exist. Note: In the future, if the PORT
   * command was specified previously to this command, the data connection
   * will be established at this point. */
  if (si->dsfd == 0) {
    reply = "425 Use PORT or PASV first.\n";
    send_all (si->csfd, (uint8_t*)reply, strlen (reply));
    return;
  }
  
  /* Merge all pathname fragments to create a single pathname to use with
   * fopen(). */
  if ((fullpath = merge_paths (si->cwd, cmd, NULL)) == NULL) {
    cleanup_stor_recv (si, NULL, 451);
    return;
  }
  
  if ((storfile = fopen (fullpath, purp)) == NULL) {
    fprintf (stderr, "%s: fopen: %s\n", __FUNCTION__, strerror (errno));
    free (fullpath);
    cleanup_stor_recv (si, NULL, 451);
  }
  free (fullpath);
  
  rv = -1;
  while ((si->cmdAbort == false) && (rv != 0)) {
    FD_ZERO (&rfds);
    FD_SET (si->dsfd, &rfds);
    timeout.tv_sec = COM_THREAD_ABORT_TIMEOUT_SEC;
    timeout.tv_usec = COM_THREAD_ABORT_TIMEOUT_USEC;

    if ((nfds = select (si->dsfd + 1, &rfds, NULL, NULL, &timeout)) == -1) {
      if (errno == EINTR)
	continue;
      fprintf (stderr, "%s: select: %s\n", __FUNCTION__, strerror (errno));
      cleanup_stor_recv (si, storfile, 451);
      return;
    }
    //check for timeout.
    if (nfds == 0)
      continue;
    
    //check if data port has rxed data
    //NOTE: Check for recv errno. Will need to add reply message.
    if (FD_ISSET (si->dsfd, &rfds)) {
      if ((rv = recv (si->dsfd, buffer, BUFFSIZE, 0)) > 0)
	fwrite (buffer, sizeof(char), rv, storfile);
    }
  }
  
  if (si->cmdAbort) {
    reply = "426 Connection closed; transfer aborted.\n";
    send_all (si->csfd, (uint8_t*)reply, strlen (reply));
    si->cmdAbort = false;
  } else {
    reply = "226 Transfer Complete.\n";
    send_all (si->csfd, (uint8_t*)reply, strlen (reply));
  }
  
  //Close the file and the data connection.
  cleanup_stor_recv (si, storfile, 0);
  return;
}


/******************************************************************************
 * cmd_retr - see "transfer.h"
 *****************************************************************************/
void cmd_retr (session_info_t *si, char *path)
{
  struct timeval timeout;
  fd_set wfds;
  FILE *retrFile;
  bool fileCheck;
  int retVal,
      selVal;
  char buffer[BUFFSIZE];
  char *aborted,
       *fullpath,
       *noAccess,
       *noConnection,
       *success,
       *transferStart;

  if (si->loggedin == false) {
    noAccess = "550 - Access denied.\n";
    send_all(si->csfd, (uint8_t *)noAccess, strlen (noAccess));
    return;
  }

  if (!(fileCheck = check_file_exist(si->cwd, path))) {
    send_mesg_553 (si->csfd);
    return;
  }

  transferStart = "150 - File status okay; about to open data connection.\n";
  send_all (si->csfd, (uint8_t *)transferStart, strlen (transferStart));

  if (si->dsfd == 0) {
    noConnection = "425 - Cannot open data connection; please use the PORT or PASV command first.\n";
    send_all(si->csfd, (uint8_t *)noConnection, strlen (noConnection));
    return;
  }

  if ((fullpath = merge_paths (si->cwd, path, NULL)) == NULL) {
    send_mesg_451 (si->csfd);
    close (si->dsfd);
    si->dsfd = 0;
    return;
  }

  if ((retrFile = fopen (fullpath, "r")) == NULL) {
    fprintf (stderr, "%s: fopen: %s\n", __FUNCTION__, strerror (errno));
    free (fullpath);
    send_mesg_451 (si->csfd);
    close (si->dsfd);
    si->dsfd = 0;
  }

  free (fullpath);
  retVal = BUFFSIZE;

  while ((si->cmdAbort == false) && (retVal != 0)) {
    FD_ZERO (&wfds);
    FD_SET (si->dsfd, &wfds);
    timeout.tv_sec = COM_THREAD_ABORT_TIMEOUT_SEC;
    timeout.tv_usec = COM_THREAD_ABORT_TIMEOUT_USEC;
    selVal = select ((si->dsfd + 1), NULL, &wfds, NULL, &timeout);


    if (selVal == -1) {
      if (errno == EINTR) {
	continue;
      }
      fprintf (stderr, "%s: select: %s\n", __FUNCTION__, strerror (errno));
      send_mesg_451 (si->csfd);
      close (si->dsfd);
      si->dsfd = 0;
      return;
    } else if (selVal == 0) {
      continue;
    }

    if (FD_ISSET (si->dsfd, &wfds)) {
      if ((retVal = fread(buffer, sizeof(*buffer), BUFFSIZE, retrFile)) == 0) {
	if (ferror (retrFile)) {
	  fprintf (stderr, "%s: fread: error while processing\n", __FUNCTION__);
	  send_mesg_451 (si->csfd);
	  close (si->dsfd);
	  si->dsfd = 0;
	  return;
	} else if (feof (retrFile)) {
	  break;
	}
      }

      if (send_all (si->dsfd, (uint8_t *)buffer, retVal) == -1) {
	send_mesg_451 (si->csfd);
	close (si->dsfd);
	si->dsfd = 0;
	return;
      }
    }
  }


  if (fclose (retrFile) == EOF) {
    fprintf (stderr, "%s: fclose: %s\n", __FUNCTION__, strerror (errno));
  }
 
  if (close (si->dsfd) == -1) {
    fprintf (stderr, "%s: close: %s\n", __FUNCTION__, strerror (errno));
  }
  
  si->dsfd = 0;

  if (si->cmdAbort == true) {
    aborted = "426 - Connection close; transfer aborted.\n";
    send_all (si->csfd, (uint8_t *)aborted, strlen (aborted));
    si->cmdAbort = false;
  } else {
    success = "226 - Closing data connection; requested file action successful.\n";
    send_all (si->csfd, (uint8_t *)success, strlen (success));
  }

  return;
}


/******************************************************************************
 * Determine if the STOR or APPE filename argument should be permanently
 * rejected.
 *
 * Arguments:
 *     si - The control connection information maintained by session().
 *   path - The pathname argument passed to the command function.
 *
 * Return values:
 *   0    Accept the filename argument.
 *  -1    Reject the filename argument.
 *****************************************************************************/
static int perm_neg_check (session_info_t *si, char *arg)
{
  int pathCheck;
  char *permdeny = "550 Permission denied.\n";

  /* If the client is anonymous or they haven't logged in, they don't have
   * permission to run this command. */
  if (si->loggedin == false || strcmp (si->user, "anonymous") == 0) {
    send_all (si->csfd, (uint8_t*)permdeny, strlen (permdeny));
    close (si->dsfd);
    si->dsfd = 0;
    return -1;
  }
  
  //Determine if the pathname argument should be accepted.
  if ((pathCheck = check_future_file (si->cwd, arg, false)) == -1) {
    cleanup_stor_recv (si, NULL, 450);
    return -1;
  } else if (pathCheck == -2) {
    cleanup_stor_recv (si, NULL, 553);
    return -1;
  } else if (pathCheck == -3) {
    cleanup_stor_recv (si, NULL, 553);
    return -1;
  }
  
  return 0;
}


/******************************************************************************
 * cleanup_stor_recv - see "transfer.h"
 *****************************************************************************/
void cleanup_stor_recv (session_info_t *si, FILE *fp,  int errcode)
{
  /* Send the appropriate reply to the client on the control connection when
   * an error has occurred. */
  if (errcode == 451) {
    send_mesg_451 (si->csfd);
  } else if (errcode == 553) {
    send_mesg_553 (si->csfd);
  } else if (errcode == 450) {
    send_mesg_450 (si->csfd);
  }
  
  //Close the filepointer if one is open.
  if (fp != NULL)
    fclose (fp);

  //Reset the data connection socket.
  close (si->dsfd);
  si->dsfd = 0;
}
