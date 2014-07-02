/******************************************************************************
 * Authors: Evan Myers, Justin Slind, Alex Tai, James Yoo
 * FTP-Server
 * Date: November 2013
 *
 * Description:
 *   The main loop that each client goes through.  Accepts commands and stores
 *   them in a queue.  Launches a separate thread to deal with commands one at
 *   a time. Handles the abort.
 *****************************************************************************/
#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include "net.h"
#include "session.h"
#include "switch.h"
#include "queue.h"


extern int shutdownServer; //Defined in the file "main.c"


/******************************************************************************
 * session - see "session.h"
 *****************************************************************************/
int session (int csfd)
{
  queue *cmdQueuePtr = NULL;
  pthread_t commandThread = 0;
  session_info_t sessioninfo;
  pthread_attr_t attr;
  char commandstr[CMD_STRLEN];
  struct timeval timeout;
  fd_set rfds;

  char *abort = "226 Abort.\n";
  
  pthread_attr_init (&attr);

  //init sessioninfo
  sessioninfo.csfd = csfd;
  sessioninfo.dsfd = 0;
  sessioninfo.cmdAbort = false;
  sessioninfo.cmdQuit = false;
  sessioninfo.loggedin = false;
  sessioninfo.cmdComplete = false;
  sessioninfo.user[0] = '\0';
  sessioninfo.cmdString[0] = '\0';
  sessioninfo.type = 'a';
  strcpy (sessioninfo.cwd, "/");
  
  commandstr[0] = '\0';
  

  //Check if the server is shutting down or if the quit cmd was received from
  //the client.
  while (!shutdownServer && !sessioninfo.cmdQuit) {
    FD_ZERO (&rfds);
    FD_SET (csfd, &rfds);
    timeout.tv_sec = SERVER_SHUTDOWN_TIMEOUT_SEC;
    timeout.tv_usec = SERVER_SHUTDOWN_TIMEOUT_USEC;
    
    //read from socket with timeout
    if (select (csfd+1, &rfds, NULL, NULL, &timeout) == -1) {
      if (errno == EINTR)
	continue;
      fprintf (stderr, "%s: select: %s\n", __FUNCTION__, strerror (errno));
      free_queue (cmdQueuePtr);
      return -1;
    }
    
    //if there's anything to read on the control socket, do so.
    if (FD_ISSET (csfd, &rfds)) {
      if (read_cmd (commandstr, csfd, &sessioninfo) == -1) {
	free_queue (cmdQueuePtr);
	return -1;
      }
      
      cmdQueuePtr = add_to_queue (commandstr, cmdQueuePtr);
    }
    
    //if command is abort (ABOR) let the current thread know
    if (strncasecmp (commandstr, "ABOR", 4) == 0) {
      sessioninfo.cmdAbort = true;
      commandstr[0] = '\0';
      send_all (sessioninfo.csfd, (uint8_t*)abort, strlen (abort));
    }
    
    
    //if there isn't a command_thread already, create one to either handle a
    //command on the command_queue or the current command
    else if (commandThread == 0 && cmdQueuePtr) {
      if (cmdQueuePtr) {
	cmdQueuePtr = pull_from_queue (commandstr, cmdQueuePtr);
      }
      
      strcpy (sessioninfo.cmdString, commandstr);
      commandstr[0] = '\0';
      if (pthread_create (&commandThread, &attr, &command_switch, (void*) &sessioninfo) == -1) {
	fprintf (stderr, "%s: pthread_create: %s\n", __FUNCTION__, strerror (errno));
	free_queue (cmdQueuePtr);
	return -1;
      }

      //check if the command thread is done, if so, join
    } else if (sessioninfo.cmdComplete) {
      if (pthread_join (commandThread, NULL) == -1) {
	fprintf (stderr, "%s: pthread_join: %s\n", __FUNCTION__, strerror (errno));
	free_queue (cmdQueuePtr);
	return -1;
      }
      commandThread = 0;
      sessioninfo.cmdString[0] = '\0';
      sessioninfo.cmdComplete = false;
    }
    //add the command to the command queue
    //else
    //cmd_queue_ptr = addToQueue(commandstr, cmd_queue_ptr);
  }
  //if shutdown or quit was given, abort the current thread if running
  sessioninfo.cmdAbort = true;
  if (commandThread) {
    if (pthread_join (commandThread, NULL) == -1) {
      fprintf (stderr, "%s: pthread_join: %s\n", __FUNCTION__, strerror (errno));
      free_queue (cmdQueuePtr);
      return -1;
    }
  }
  
  //Close the data connection socket.
  if (sessioninfo.dsfd > 0) {
    if (close (sessioninfo.dsfd) == -1)
      fprintf (stderr, "%s: close: %s\n", __FUNCTION__, strerror (errno));
  }
  
  free_queue (cmdQueuePtr);
  pthread_attr_destroy (&attr);
  return 0;
}


/******************************************************************************
 * read_cmd - see "session.h"
 *****************************************************************************/
int read_cmd (char *str, int sock, session_info_t *si)
{
  int rt = 0;
  int len = 0;
  
  //keep adding rxed chars to str until \n rxed
  while (1) {
    if ((rt = recv (sock, str+len, 1, 0)) == -1) {
      if (errno == EINTR)
	continue;
      fprintf (stderr, "%s: recv: %s\n", __FUNCTION__, strerror (errno));
      return -1;
    }

    if (rt > 0) {      
      len += rt;
      if (str[len-1] == '\n') {
	str[len-1] = '\0'; //null terminate string
	break;
      }
    } else if (rt == 0) {
      si->cmdAbort = true;
      si->cmdQuit = true;
      break;
    }
  }
  
  return len;
}
