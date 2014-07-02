/******************************************************************************
 * Authors: Evan Myers, Justin Slind, Alex Tai, James Yoo
 * FTP-Server
 * Date: November 2013
 *
 * Description:
 *   Functions that read commands from standard input while the server is
 *   running, and perform the received commands.
 *****************************************************************************/
#ifndef __SERVERCMD_H__
#define __SERVERCMD_H__


/* Inform main() to shutdown the server. This number may be changed freely, but
 * the replacement MUST be negative and MUST not interfere with errno (-1). */
#define SHUTDOWN_SERVER -999 


/******************************************************************************
 * Display a welcome message, connection information, and how to view a list
 * of commands on server startup.
 *****************************************************************************/
int welcome_message (void);


/******************************************************************************
 * Read the command that was entered on standard input on the server console.
 * Redirect the command that was read to the appropriate function if
 * applicable.
 *
 * Return values:
 *   0    success
 *  -1    error, the command was not read
 *****************************************************************************/
int read_server_cmd (void);


#endif //__SERVERCMD_H__
