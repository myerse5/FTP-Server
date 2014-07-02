/******************************************************************************
 * Authors: Evan Myers, Justin Slind, Alex Tai, James Yoo
 * FTP-Server
 * Date: November 2013
 *
 * Description:
 *   A massive 'if-else' statement that determines the command that has been
 *   invoked by the client and performs an appropriately related action.
 *****************************************************************************/
#ifndef __SWITCH_H__
#define __SWITCH_H__


/******************************************************************************
 * When the control thread receives a command from the client, the command is
 * then passed to this function. The command will be checked for validity, and
 * passed to the appropriate function to perform the desired task.
 *
 * This function will always be called as the function pointer argument to
 * pthread_create(). Therefore, the 'void *' return value and parameter should
 * not be altered without thought.
 *
 * Arguments:
 *   param - The command received from the client on the control connection.
 *****************************************************************************/
void *command_switch (void *param);


#endif //__SWITCH_H__
