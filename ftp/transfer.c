/******************************************************************************
 * Students: Evan Myers, Justin Slind, Alex Tai, James Yoo
 * Course: CMPT-361
 * Assignment #3 - ftp server
 * File: transfer.c
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


//TODO Consolidate and check this arbitrary value.
#define BUFFSIZE 1000


/******************************************************************************
 * cmd_stou - see "cmd_stor.h"
 *****************************************************************************/
void cmd_stou (session_info_t *si, char *arg) {
  //if client is anonymous or they haven't logged in, they don't
  //have permission to run this command
  if (si->logged_in == false || strcmp(si->user,"anonymous") == 0) {
    char *permdeny = "550 Permission denied.\n";
    send_all(si->c_sfd,(uint8_t*)permdeny,strlen(permdeny));
    close(si->d_sfd);
    si->d_sfd = 0;
    return;
  }

  //RFC 959 does not require or expect parameter for STOU cmd.
  //This implementation ignores parameter to allow
  //modern ftp clients to work with STOU.
  srand(time(NULL));
  char tempname[256];
  char *fullPath = NULL;
  int rt;
  do {
    if (fullPath)
      free(fullPath);
    sprintf(tempname,"%d",rand());
    fullPath = merge_paths(si->cwd,tempname,NULL);
    rt = access(fullPath, F_OK);
  } while ((rt != -1));          //verify that random name doesn't exist
  free(fullPath);
  store(si,tempname,"w");
  
  return;
}


/******************************************************************************
 * cmd_stor - see "transfer.h"
 *****************************************************************************/
void cmd_stor(session_info_t *si, char *cmd) {
  if (perm_neg_check (si, cmd) == -1)
    return;
  
  store(si,cmd,"w");
  return;
}


/******************************************************************************
 * cmd_appe - see "transfer.h"
 *****************************************************************************/
void cmd_appe (session_info_t *si, char *cmd) {
  if (perm_neg_check (si, cmd) == -1)
    return;
  
  store(si,cmd,"a");
  return;
}


/******************************************************************************
 * Stores or appends a file to the servers filesystem, which action is
 * performed is determined by the 'purp' argument.
 *
 * Note: The purp argument characters are equivelant to the second argument to
 *       fopen(). See the fopen() manpage for a full list of options. Read
 *       options should not be passed to this function.
 *
 * Arguments:
 *   cmd - current command with parameter
 *    si - info for current session
 *  purp - "purpose", a character to be used with fopen, write or append.
 *
 * Original author: Justin Slind
 * Rewritten by: Evan Myers
 *****************************************************************************/
static void store (session_info_t *si, char *cmd, char *purp) {
  struct timeval timeout;
  fd_set rfds;
  FILE *storfile;
  int rt = -1;
  char buffer[BUFFSIZE];
  
  //send positive prelimitary reply
  char *transferstart = "150 Opening ";
  char *middle = " mode data connection for ";
  char *type;
  send_all(si->c_sfd,(uint8_t*)transferstart,strlen(transferstart));
  if (si->type == 'a')
    type = "ASCII";
  else
    type = "BINARY";
  send_all(si->c_sfd,(uint8_t*)type,strlen(type));
  send_all(si->c_sfd,(uint8_t*)middle,strlen(middle));
  send_all(si->c_sfd,(uint8_t*)cmd,strlen(cmd));
  char *endln = ".\n";
  send_all(si->c_sfd,(uint8_t*)endln,strlen(endln));
  
  //data connection must already exist
  if (si->d_sfd == 0) {
    char *nodata = "425 Use PORT or PASV first.\n";
    send_all(si->c_sfd,(uint8_t*)nodata,strlen(nodata));
    return;
  }
  
  /* Merge all pathname fragments to create a single pathname to use with
   * fopen(). */
  char *fullPath;
  if ((fullPath = merge_paths(si->cwd, cmd, NULL)) == NULL) {
    cleanup_stor_recv (si, NULL, 451);
    return;
  }
  
  if ((storfile = fopen(fullPath,purp)) == NULL) {
    fprintf (stderr, "%s: fopen: %s\n", __FUNCTION__, strerror (errno));
    free (fullPath);
    cleanup_stor_recv (si, NULL, 451);
  }
  free(fullPath);
  
  while(si->cmd_abort == false && rt != 0) {
    FD_ZERO(&rfds);
    FD_SET(si->d_sfd,&rfds);
    timeout.tv_sec = COM_THREAD_ABORT_TIMEOUT_SEC;
    timeout.tv_usec = COM_THREAD_ABORT_TIMEOUT_USEC;
    int sr = 0;
    if ((sr = select(si->d_sfd+1,&rfds,NULL,NULL,&timeout)) == -1) {
      if (errno == EINTR)
	continue;
      fprintf (stderr, "%s: select: %s\n", __FUNCTION__, strerror (errno));
      cleanup_stor_recv (si, storfile, 451);
      return;
    }
    //check for timeout.
    if (sr == 0)
      continue;
    
    //check if data port has rxed data
    if (FD_ISSET(si->d_sfd, &rfds)) {
      if ((rt = recv(si->d_sfd,buffer,BUFFSIZE,0)) > 0) {
	fwrite(buffer,sizeof(char),rt,storfile);
      }
    }
  }
  
  if (si->cmd_abort) {
    char *aborted = "426 Connection closed; transfer aborted.\n";
    send_all(si->c_sfd,(uint8_t*)aborted,strlen(aborted));
    si->cmd_abort = false;
  } else {
    char *success = "226 Transfer Complete.\n";
    send_all(si->c_sfd,(uint8_t*)success,strlen(success));
  }
  
  //close file and data connection
  cleanup_stor_recv (si, storfile, 0);
  return;
}


/******************************************************************************
 * command_retrieve - see "transfer.h"
 *****************************************************************************/
void command_retrieve (session_info_t *si, char *path) {
  struct timeval timeout;
  fd_set wfds;
  FILE *retrFile;
  bool fileCheck;
  int retVal,
      selVal;
  char buffer[BUFFSIZE];
  char *aborted,
       *fullPath,
       *noAccess,
       *noConnection,
       *success,
       *transferStart;

  if (si->logged_in == false) {
    noAccess = "550 - Access denied.\n";
    send_all(si->c_sfd, (uint8_t *)noAccess, strlen(noAccess));
    return;
  }

  if (!(fileCheck = check_file_exist(si->cwd, path))) {
    send_mesg_553 (si->c_sfd);
    return;
  }

  transferStart = "150 - File status okay; about to open data connection.\n";
  send_all(si->c_sfd, (uint8_t *)transferStart, strlen(transferStart));

  if (si->d_sfd == 0) {
    noConnection = "425 - Cannot open data connection; please use the PORT or PASV command first.\n";
    send_all(si->c_sfd, (uint8_t *)noConnection, strlen(noConnection));
    return;
  }

  if ((fullPath = merge_paths(si->cwd, path, NULL)) == NULL) {
    send_mesg_451 (si->c_sfd);
    close(si->d_sfd);
    si->d_sfd = 0;
    return;
  }

  if ((retrFile = fopen(fullPath, "r")) == NULL) {
    fprintf (stderr, "%s: fopen: %s\n", __FUNCTION__, strerror (errno));
    free(fullPath);
    send_mesg_451 (si->c_sfd);
    close (si->d_sfd);
    si->d_sfd = 0;
  }

  free(fullPath);
  retVal = BUFFSIZE;

  while ((si->cmd_abort == false) && (retVal != 0)) {
    FD_ZERO(&wfds);
    FD_SET(si->d_sfd, &wfds);
    timeout.tv_sec = COM_THREAD_ABORT_TIMEOUT_SEC;
    timeout.tv_usec = COM_THREAD_ABORT_TIMEOUT_USEC;
    selVal = select((si->d_sfd + 1), NULL, &wfds, NULL, &timeout);


    if (selVal == -1) {

      if (errno == EINTR) {
	continue;
      }

      fprintf(stderr, "%s: select: %s\n", __FUNCTION__, strerror(errno));
      send_mesg_451(si->c_sfd);
      close(si->d_sfd);
      si->d_sfd = 0;
      return;
    } else if (selVal == 0) {
      continue;
    }

    if (FD_ISSET(si->d_sfd, &wfds)) {

      if ((retVal = fread(buffer, sizeof(*buffer), BUFFSIZE, retrFile)) == 0) {
	
	if (ferror (retrFile)) {
	  fprintf (stderr, "%s: fread: error while processing\n", __FUNCTION__);
	  send_mesg_451(si->c_sfd);
	  close(si->d_sfd);
	  si->d_sfd = 0;
	  return;
	} else if (feof(retrFile)) {
	  break;
	}

      }

      if (send_all(si->d_sfd, (uint8_t *)buffer, retVal) == -1) {
	send_mesg_451(si->c_sfd);
	close(si->d_sfd);
	si->d_sfd = 0;
	return;
      }

    }

  }


  if (fclose(retrFile) == EOF) {
    fprintf (stderr, "%s: fclose: %s\n", __FUNCTION__, strerror (errno));
  }
 
  if (close(si->d_sfd) == -1) {
    fprintf (stderr, "%s: close: %s\n", __FUNCTION__, strerror (errno));
  }
  
  si->d_sfd = 0;

  if (si->cmd_abort == true) {
    aborted = "426 - Connection close; transfer aborted.\n";
    send_all(si->c_sfd, (uint8_t *)aborted, strlen(aborted));
    si->cmd_abort = false;
  } else {
    success = "226 - Closing data connection; requested file action successful.\n";
    send_all(si->c_sfd, (uint8_t *)success, strlen(success));
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
 *
 * Original author: Evan Myers
 *****************************************************************************/
static int perm_neg_check (session_info_t *si, char *arg)
{
  int pathCheck;
  //if client is anonymous or they haven't logged in, they don't
  //have permission to run this command
  if (si->logged_in == false || strcmp(si->user,"anonymous") == 0) {
    char *permdeny = "550 Permission denied.\n";
    send_all(si->c_sfd,(uint8_t*)permdeny,strlen(permdeny));
    close(si->d_sfd);
    si->d_sfd = 0;
    return -1;
  }
  
  //Determine if the pathname argument should be accepted.
  if ((pathCheck = check_futer_file(si->cwd, arg, false)) == -1) {
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
   * an error has occured. */
  if (errcode == 451) {
    send_mesg_451 (si->c_sfd);
  } else if (errcode == 553) {
    send_mesg_553 (si->c_sfd);
  } else if (errcode == 450) {
    send_mesg_450 (si->c_sfd);
  }
  
  //Close the filepointer if one is open.
  if (fp != NULL)
    fclose (fp);

  //Reset the data connection socket.
  close (si->d_sfd);
  si->d_sfd = 0;
}

