/******************************************************************************
 * Students: Evan Myers, Justin Slind, Alex Tai, James Yoo
 * Course: CMPT-361
 * Assignment #3 - ftp server
 * File: main.c
 * Date: November 2013
 *
 * Description:
 *    The server begins and ends here. When a control connection is established
 *    with a client, control of all futer interactions between the server and
 *    this client is passed to the function control_thread() as a pthread.
 *
 * Acknowledgements:
 *    Evan - I have included my acknowledgements for working with pthreads in
 *           the file header of "ctrlthread.h" and "ctrlthread.c".
 *
 *    Evan - For debugging response code messages and orders, I have referred
 *           to this webpage:   http://cr.yp.to/ftp.html
 *
 * Compatible programs:
 *     -netcat (nc)
 *     -ftp program
 *     -filezilla
 *     -web browser - tested with "Firefox 25.0: Mozilla Firefox for Ubuntu
 *                    canonical - 1.0"
 *     -others which have not been tested.
 *
 * Note:
 *     Our server uses a server configuration file and a user file. These files
 *     may be found with the names "ftp.conf" and "user.conf" respectively.
 *****************************************************************************/
#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "config.h"
#include "ctrlthread.h"
#include "net.h"
#include "servercmd.h"


/******************************************************************************
 * Global variables which each thread can access and/or modify.
 *****************************************************************************/
/* One control thread is created for every control connection made with a 
 * client. This counter is modified by main(), and all control threads, so it
 * has been protected from multiple read/writes occuring at the same time by a
 *  mutex. */
pthread_mutex_t ctrl_count_mutex = PTHREAD_MUTEX_INITIALIZER;
int active_control_threads = 0;


/******************************************************************************
 * Global variables which should only be read by functions that are not main().
 *****************************************************************************/
/* Threads monitor this variable, when main() sets this value to TRUE, all
 * threads will terminate themselves after closing open sockets and freeing
 * heap memory. This variable is only modified by main(). */
int shutdown_server = false;


/* The root directory of the server. When a new control connection is accepted,
 * this is the current working directory of the client. The client will not be
 * able to move to any ancestor directory of this root directory for the
 * reasons listed below:
 *
 *   -The current working directory for each client is stored in two variables
 *    and merged together when required.
 *         -The first portion of the cwd is the string stored in the global
 *          variable below.
 *         -The second portion of the cwd is found in the session_info_t
 *          structure that is created for each control connection.
 *
 *   -When a client command requires a full pathname, such as when storing or
 *    retreiving a file, the cwd of the session is appended to the global root
 *    directory.
 *
 *   -Before appending the session cwd to the root cwd, any relative directory
 *    path entries (eg. '.' and '..') are first resolved, and if this path
 *    is a directory which is an ancestor to the global root directory, the
 *    command is not accepted. */
char *rootdir;


/******************************************************************************
 * main
 *
 * Original author: Justin Slind
 * Updated/rewritten by: Evan Myers
 *****************************************************************************/
int main (int argc, char *argv[])
{
  int listen_sfd;      //Listen for control connections on this sfd.
  int *c_sfd;          //An accepted control connection sfd.
  pthread_t thread;    //The handle for a new thread.
  pthread_attr_t attr; //pthread attribute, to set detached state on creation.
  
  char *root_temp;

  //Retrieve the name of the root directory from the config file.
  if ((root_temp = get_config_value ("ROOT_PATH_CONFIG", FTP_CONFIG_FILE)) == NULL)
    return -1;

  /* Append the relative path from the server executable to the server root directory
   * to the absolute path of the server executable. */
  if ((rootdir = get_config_path (root_temp)) == NULL) {
    free (root_temp);
    return -1;
  }
  free (root_temp);

  root_temp = rootdir;
  /* Canonicalize the path to the server root directory 
   * (eg. resolve all "..", ".", excessive "/" and symbolic links). */
  if ((rootdir = canonicalize_file_name (root_temp)) == NULL) {
    fprintf (stderr, "%s: canonicalize_file_name: %s\n", __FUNCTION__, strerror (errno));
  }
  free (root_temp);

  //Initialize the pthread attributes.
  if (pthread_attr_init (&attr) != 0) {
    fprintf (stderr, "%s: pthread_attr_init: %s\n", __FUNCTION__, strerror (errno));
    return -1;
  }

  //Set the detach state attribute.
  if (pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) != 0) {
    fprintf (stderr, "%s: pthread_attr_init: %s\n", __FUNCTION__, strerror (errno));
    return -1;
  }

  //Create a socket to listen for control connections.
  if ((listen_sfd = get_control_sock ()) == -1)
    return -1;

  //Display usage instructions to the server operator, and connection information.
  if (welcome_message() == -1) {
    return -1;
  }

  /* This loop does the following:
   *   -Accepts a control connection from a client.
   *   -Starts a thread for the accepted control connection.
   *   OR
   *   -Read a command from stdin entered on the server console.
   *   -Call server_cmd_handler to perform the command.
   *
   * This loop will exit when:
   *    -malloc cannot allocate memory for an integer.
   *    -The functions pthread_mutex_lock or pthread_mutex_unlock return error.
   *    -The command "shutdown" is entered on the server console. */ 
  while (1) {
    if ((c_sfd = malloc (sizeof (*c_sfd))) == NULL) {
      fprintf (stderr, "%s: malloc: could not allocate the required space\n",
	       __FUNCTION__);
      break;
    }

    //Accept a connection from the client, or read a server command on stdin.
    if ((*c_sfd = accept_connection (listen_sfd, ACCEPT_CONTROL, NULL)) == -1) {
      free (c_sfd);
      continue;
    } 
    else if (*c_sfd == STDIN_READY) {   //There is something to read on stdin.
      if (read_server_cmd () == SHUTDOWN_SERVER) {
	shutdown_server = true;
	free (c_sfd);
	break;
      } else {
	free (c_sfd);
	continue;
      }
    }

    //Create a new thread for this control connection.
    if (pthread_create (&thread, &attr, &control_thread, c_sfd) != 0) {
      fprintf (stderr, "%s: pthread_create: %s\n", __FUNCTION__, strerror (errno));
      free (c_sfd);
      continue;
    }

    //Increment the control connection thread count.
    if (modify_cthread_count (1) == -1)
      break;
  }

  free (rootdir);

  if (active_control_threads > 0)
    printf ("waiting on threads to resolve...\n");

  //Wait for the control threads to shutdown.
  while (active_control_threads > 0) {
    sleep (1);
  }

  if (pthread_attr_destroy(&attr) == -1)
    fprintf (stderr, "%s: pthread_attr_destroy: %s\n", __FUNCTION__, strerror (errno));

  printf ("All threads have terminated, exiting the program.\n");
  return 0;
}
