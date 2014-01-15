/******************************************************************************
 * Students: Evan Myers, Justin Slind, Alex Tai, James Yoo
 * Course: CMPT-361
 * Assignment #3 - ftp server
 * File: ctrlthread.h
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
#ifndef __CTRLTHREAD_H__
#define __CTRLTHREAD_H__


//Defined in "main.c"
extern int active_control_threads;      //Used to lock active_control_threads.
extern pthread_mutex_t ctrl_count_mutex;//Number of control connection threads.


/******************************************************************************
 * Original author: Justin Slind
 * Updated by: Evan Myers
 *****************************************************************************/
void *control_thread (void * arg);


/******************************************************************************
 * Increase or decrease the number of active control connection threads.This
 * function uses a mutex to lock the count before retrieving the value. This
 * will ensure two threads do not try to modify the variable at the same time,
 * whith results being undefined.
 *
 * Arguments:
 *  value - This function will treat this argument as either zero, negative,
 *          or posotive. It will modify the count by 0, -1, or +1 respectively.
 *
 * Return values:
 *   0    Success
 *  -1    Error (the mutex locking functions failed)
 *
 * Original author: Evan Myers
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
 *
 * Original author: Evan Myers
 *****************************************************************************/
int get_cthread_count (void);


#endif //__CTRLTHREAD_H__
