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
#ifndef __CTRLTHREAD_H__
#define __CTRLTHREAD_H__


//Defined in "main.c"
extern int activeControlThreads;       //Used to lock active_control_threads.
extern pthread_mutex_t ctrlCountMutex; //Number of control connection threads.


/******************************************************************************
 * When main() accepts a new connection, a pthread is started with this
 * this wrapper function. A welcome message is sent to the user, and a new
 * session is began by calling the function session(). When the session has
 * been terminated, control is passed back to this function where all remaining
 * heap memory is freed, and the control socket closed.
 *
 * Arguments:
 *    arg - The control socket file descriptor.
 *****************************************************************************/
void *control_thread (void * arg);


/******************************************************************************
 * Increase or decrease the number of active control connection threads.This
 * function uses a mutex to lock the count before retrieving the value. This
 * will ensure two threads do not try to modify the variable at the same time,
 * with results being undefined.
 *
 * Arguments:
 *  value - This function will treat this argument as either zero, negative,
 *          or positive. It will modify the count by 0, -1, or +1 respectively.
 *
 * Return values:
 *   0    Success
 *  -1    Error (the mutex locking functions failed)
 *****************************************************************************/
int modify_cthread_count (int value);


/******************************************************************************
 * Get the number of active control connection threads. This function uses a
 * mutex to lock the count before retrieving the value. This will ensure the
 * value is not being modified while being collected.
 *
 * Return values:
 *   >=0  The number of active control connection threads.
 *    <0  Error (the count was negative or the mutex locking functions failed)
 *****************************************************************************/
int get_cthread_count (void);


#endif //__CTRLTHREAD_H__
