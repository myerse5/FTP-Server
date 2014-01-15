/******************************************************************************
 * Students: Evan Myers, Justin Slind, Alex Tai, James Yoo
 * Course: CMPT-361
 * Assignment #3 - ftp server
 * File: servercmd.c
 * Date: November 2013
 *
 * Description:
 *   Functions that read commands from standard input while the server is
 *   running, and perform the recieved commands.
 *****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "ctrlthread.h"
#include "net.h"
#include "servercmd.h"


#define MAX_SERVER_CMD_SZ 81 //Standard terminal window size.


/******************************************************************************
 * local function prototypes
 *****************************************************************************/
static int server_info (void);
static void print_help (void);


/******************************************************************************
 * welcome_message - see servercmd.h
 *****************************************************************************/
int welcome_message (void)
{
  printf ("The server is now ready to accept client connections.\n\n");

  //Display the connection information.
  if (server_info () == -1)
    return -1;

  printf ("\nYou may enter commands to this console. ");
  printf ("Enter \"help\" for a list of commands.\n");
  return 0;
}


/******************************************************************************
 * Display the information necessary for a client to connect to the server.
 *
 * Return values:
 *   0    success
 *  -1    error
 *
 * Original author: Evan Myers
 *****************************************************************************/
static int server_info (void)
{
  
  char *interface;
  char address[INET_ADDRSTRLEN];
  char *port;

  //Get the chosen interface from the server configuration file.
  if ((interface = get_config_value ("INTERFACE_CONFIG",
				     FTP_CONFIG_FILE)) == NULL)
    return -1;

  //Get the IPv4 address for the interface.
  if (get_interface_address (interface, &address) == -1) {
    free (interface);
    return -1;
  }
  free (interface);

  //Get the chosen port from the server configuration file.
  if ((port = get_config_value ("DEFAULT_PORT_CONFIG",
				FTP_CONFIG_FILE)) == NULL)
    return -1;

  //Print the results to the console.
  printf ("The server can be reached at:\n");
  printf ("\tIP address: %s\n", address);
  printf ("\ton port   : %s\n", port);
 
  free (port);
  return 0;
}


/******************************************************************************
 * Display a list of server commands to the server operator.
 *
 * Original author: Evan Myers
 *****************************************************************************/
static void print_help (void)
{
  printf ("The current commands are:\n");
  printf ("\tclients\n");
  printf ("\thelp\n");
  printf ("\tserverinfo\n");
  printf ("\tshutdown\n");
  return;
}


/******************************************************************************
 * read_server_cmd - see servercmd.h
 *****************************************************************************/
int read_server_cmd (void)
{
  char cmd[MAX_SERVER_CMD_SZ];

  //Read a command from standard input.
  if (fgets (cmd, MAX_SERVER_CMD_SZ - 1, stdin) == NULL)
    return -1;

  if (strcmp (cmd, "help\n") == 0) {
    print_help ();
    return 0;
 
  } else if (strcmp (cmd, "serverinfo\n") == 0) {
    if (server_info () == -1)
      return -1;
    return 0;

  } else if (strcmp (cmd, "shutdown\n") == 0) {
    return SHUTDOWN_SERVER;

  } else if (strcmp (cmd, "clients\n") == 0) {
    printf ("Current number of clients: %d\n", get_cthread_count());

  } else {
    printf ("Command not recognized, enter \"help\" for a list of commands.\n");
  }

  return 0;
}
