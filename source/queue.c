/******************************************************************************
 * Students: Evan Myers, Justin Slind, Alex Tai, James Yoo
 * Course: CMPT-361
 * Assignment #3 - ftp server
 * File: queue.c
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
 * addToQueue - see "queue.h"
 *****************************************************************************/
queue* addToQueue (char *commandstr, queue *cmd_queue_ptr) {
  queue *temp;
  queue *newptr = malloc (sizeof (queue));
  if (!newptr) {
    fprintf (stderr, "%s: malloc: could not allocate the required space\n", __FUNCTION__);
    return cmd_queue_ptr;
  }
  strcpy(newptr->cmd,commandstr);
  newptr->next = NULL;
  
  
  if (cmd_queue_ptr) {
    temp = cmd_queue_ptr;
    //Iterate to the end of the queue.
    while(temp->next)
      temp = temp->next;
    //Add node to the end of the queue.
    temp->next = newptr;
  } else {
    //Otherwise node is the head of the queue.
    cmd_queue_ptr = newptr; 
  }
  
  return cmd_queue_ptr;
}


/******************************************************************************
 * pullFromQueue - see "queue.h"
 *****************************************************************************/
queue* pullFromQueue(char *commandstr, queue *cmd_queue_ptr) {
  if (cmd_queue_ptr) {
    queue *tempptr = cmd_queue_ptr->next;
    strcpy(commandstr, cmd_queue_ptr->cmd);
    free(cmd_queue_ptr);
    return tempptr;
  } else {
    return NULL;
  }
}


/******************************************************************************
 * freeQueue - see "queue.h"
 *****************************************************************************/
void freeQueue (queue *cmd_queue_ptr) {
  if (cmd_queue_ptr) {
    if (cmd_queue_ptr->next) {
      freeQueue(cmd_queue_ptr->next);
      cmd_queue_ptr->next = NULL;
    }
    free(cmd_queue_ptr);
  }
  return;
}

