/******************************************************************************
 * FTP-Server
 * Authors: Evan Myers, Justin Slind, Alex Tai, James Yoo
 * Date: November 2013
 *
 * Description:
 *    This file contains the functions that start a thread for an established
 *    control connection with a client. Global variables that are modified
 *    by these threads are done so by functions found here.
 *****************************************************************************/
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "ctrlthread.h"
#include "reply.h"
#include "session.h"


/******************************************************************************
 * control_thread - see ctrlthread.h
 *****************************************************************************/
void *control_thread (void *arg) {
  int *csfd = arg;    //Control socket file descriptor.

  //Send the welcome message to the client.
  while (send_mesg_220 (*csfd) != 0);

  //Transfer control of the thread to session() to perform user commands.
  session (*csfd);

  //Free all heap memory. Close sockets which are no longer required.
  close (*csfd);
  free (csfd);

  //Decrement the active control thread count and terminate the thread.
  modify_cthread_count (-1);
  return NULL;
}

/******************************************************************************
 * modify_cthread_count - see ctrlthread.h
 *****************************************************************************/
int modify_cthread_count (int value)
{
  if (value == 0)
    return 0;

  //Lock the mutex.
  if (pthread_mutex_lock (&ctrlCountMutex) != 0) {
    fprintf (stderr, "%s: pthread_mutex_lock error\n", __FUNCTION__);
    return -1;
  }

  //Modify the count, only one thread can be added or removed at one time.
  if (value > 0)
    activeControlThreads++;
  else
    activeControlThreads--;

  //Unlock the mutex.
  if (pthread_mutex_unlock (&ctrlCountMutex) != 0) {
    fprintf (stderr, "%s: pthread_mutex_unlock error\n", __FUNCTION__);
    return -1;
  }

  return 0;
}


/******************************************************************************
 * get_cthread_count - see ctrlthread.h
 *****************************************************************************/
int get_cthread_count (void)
{
  int count;

  //Lock the mutex.
  if (pthread_mutex_lock (&ctrlCountMutex) != 0) {
    fprintf (stderr, "%s: pthread_mutex_lock error\n", __FUNCTION__);
    return -1;
  }

  count = activeControlThreads;

  //Unlock the mutex.
  if (pthread_mutex_unlock (&ctrlCountMutex) != 0) {
    fprintf (stderr, "%s: pthread_mutex_unlock error\n", __FUNCTION__);
    return -1;
  }

  //The count should never be negative. Treat negative values as an error.
  return count;
}
