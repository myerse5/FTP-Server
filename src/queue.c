/******************************************************************************
 * FTP-Server
 * Authors: Evan Myers, Justin Slind, Alex Tai, James Yoo
 * Date: November 2013
 *
 * Description:
 *   Functions to support building a queue of commands, stored as strings,
 *   first in first out.
 *****************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "queue.h"


/******************************************************************************
 * add_to_queue - see "queue.h"
 *****************************************************************************/
queue* add_to_queue (char *commandstr, queue *cmdQueuePtr)
{
  queue *temp;
  queue *newptr = malloc (sizeof (queue));
  if (!newptr) {
    fprintf (stderr, "%s: malloc of %lu bytes failed\n", __FUNCTION__, sizeof(queue));
    return cmdQueuePtr;
  }
  strcpy (newptr->cmd, commandstr);
  newptr->next = NULL;
  
  
  if (cmdQueuePtr) {
    temp = cmdQueuePtr;
    // Iterate to the end of the queue.
    while(temp->next)
      temp = temp->next;
    // Add node to the end of the queue.
    temp->next = newptr;
  } else {
    // Otherwise node is the head of the queue.
    cmdQueuePtr = newptr; 
  }
  
  return cmdQueuePtr;
}


/******************************************************************************
 * pull_from_queue - see "queue.h"
 *****************************************************************************/
queue* pull_from_queue (char *commandstr, queue *cmdQueuePtr)
{
  queue *tempptr;

  if (cmdQueuePtr) {
    tempptr = cmdQueuePtr->next;
    strcpy (commandstr, cmdQueuePtr->cmd);
    free (cmdQueuePtr);
    return tempptr;
  } else {
    return NULL;
  }
}


/******************************************************************************
 * free_queue - see "queue.h"
 *****************************************************************************/
void free_queue (queue *cmdQueuePtr) {
  if (cmdQueuePtr) {
    if (cmdQueuePtr->next) {
      free_queue (cmdQueuePtr->next);
      cmdQueuePtr->next = NULL;
    }
    free (cmdQueuePtr);
  }
  return;
}

