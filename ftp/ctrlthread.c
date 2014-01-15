/******************************************************************************
 * Students: Evan Myers, Justin Slind, Alex Tai, James Yoo
 * Course: CMPT-361
 * Assignment #3 - ftp server
 * File: ctrlthread.c
 * Date: November 2013
 *
 * Description:
 *    This file contains the functions that start a thread for an established
 *    control connection with a client. Global variables that are modified
 *    by these threads are done so by functions found here.
 *
 * Acknowledgements:
 *    Evan - I learned how to use pthreads with these sections of this tutorial
 *           website, and of course many manpages.
 *           https://computing.llnl.gov/tutorials/pthreads/#CreatingThreads
 *           https://computing.llnl.gov/tutorials/pthreads/#Joining
 *           https://computing.llnl.gov/tutorials/pthreads/#Mutexes
 *
 *           I also viewed this website to clarify the detached state.
 *           http://stackoverflow.com/a/6043021
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
  int *c_sfd = arg;

  //Send the welcome message to the client.
  while (send_welcome_mesg_220 (*c_sfd) != 0);

  //Transfer control of the thread to session() to perform user commands.
  session (*c_sfd);

  //Free all heap memory. Close sockets which are no longer required.
  close (*c_sfd);
  free (c_sfd);

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
  if (pthread_mutex_lock (&ctrl_count_mutex) != 0) {
    fprintf (stderr, "%s: pthread_mutex_lock error\n", __FUNCTION__);
    return -1;
  }

  //Modify the count, only one thread can be added or removed at one time.
  if (value > 0)
    active_control_threads++;
  else
    active_control_threads--;

  //Unlock the mutex.
  if (pthread_mutex_unlock (&ctrl_count_mutex) != 0) {
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
  if (pthread_mutex_lock (&ctrl_count_mutex) != 0) {
    fprintf (stderr, "%s: pthread_mutex_lock error\n", __FUNCTION__);
    return -1;
  }

  count = active_control_threads;

  //Unlock the mutex.
  if (pthread_mutex_unlock (&ctrl_count_mutex) != 0) {
    fprintf (stderr, "%s: pthread_mutex_unlock error\n", __FUNCTION__);
    return -1;
  }

  //The count should never be negative. Treat negative values as an error.
  return count;
}
