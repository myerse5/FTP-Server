/******************************************************************************
 * FTP-Server
 * Authors: Evan Myers, Justin Slind, Alex Tai, James Yoo
 * Date: November 2013
 *
 * Description:
 *   Functions to support building a queue of commands, stored as strings,
 *   first in first out.
 *****************************************************************************/
#ifndef __QUEUE_H__
#define __QUEUE_H__


//TODO remove magic number.
typedef struct node {
  char cmd[4096];
  struct node *next;
} queue;


/******************************************************************************
 * Adds a command to the end of the queue.
 *
 * Arguments:
 *     commandstr - Null terminated string holding command and parameter.
 *   *cmdQueuePtr - Head of the queue.
 *
 * Returns
 * 	Head of the queue.
 *****************************************************************************/
queue* add_to_queue (char *commandstr, queue *cmdQueuePtr);


/******************************************************************************
 * Pulls a command from the front of the queue.
 *
 * Arguments:
 *    commandstr - Null terminated string holding command and parameter.
 *   cmdQueuePtr - Head of the queue
 *
 * Returns
 *   Queue* - The next command to be processed is returned.
 *   NULL - There was nothing in the queue to return.
 *****************************************************************************/
queue* pull_from_queue (char *commandstr, queue *cmdQueuePtr);


/******************************************************************************
 * Frees allocated memory in the queue.
 *
 * Arguments:
 *   cmdQueuePtr - Head of the queue.
 *****************************************************************************/
void free_queue (queue *cmdQueuePtr);


#endif //__QUEUE_H__
