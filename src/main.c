/******************************************************************************
 * FTP-Server
 * Authors: Evan Myers, Justin Slind, Alex Tai, James Yoo
 * Date: November 2013
 *
 * Description:
 *    The server begins and ends here. When a control connection is established
 *    with a client, control of all future interactions between the server and
 *    this client is passed to the function control_thread() as a pthread.
 *
 * Compatible programs:
 *     -netcat (nc)
 *     -ftp program
 *     -Filezilla
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
 * has been protected from multiple read/writes occurring at the same time by a
 *  mutex. */
pthread_mutex_t ctrlCountMutex = PTHREAD_MUTEX_INITIALIZER;
int activeControlThreads = 0;


/******************************************************************************
 * Global variables which should only be read by functions that are not main().
 *****************************************************************************/
/* Threads monitor this variable, when main() sets this value to TRUE, all
 * threads will terminate themselves after closing open sockets and freeing
 * heap memory. This variable is only modified by main(). */
int shutdownServer = false;


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
 *    retrieving a file, the cwd of the session is appended to the global root
 *    directory.
 *
 *   -Before appending the session cwd to the root cwd, any relative directory
 *    path entries (eg. '.' and '..') are first resolved, and if this path
 *    is a directory which is an ancestor to the global root directory, the
 *    command is not accepted. */
char *rootdir;

/******************************************************************************
 * Locations that need to grab info from the config file. TODO.
 *   - main.c, main(), [rootdir]
 *   - net.c, get_control_sock(), [port, iface]
 *   - net.c, get_interface_address() -> move to global IP address variable.
 *     (do this only once while/directly after reading config file).
 *   - net.c, cmd_pasv(), [iface]
 *   - 
 *****************************************************************************/

/******************************************************************************
 * Clean up weird strings and whatnot. TODO
 *     FILE --- FUNCTION ---- First offending variable declaration.
 *    - net.c, cmd_port(), char *portsuccess.
 *    - directory.c, makeDir(), char * printStart.
 *    - help.c, command_help(), int strLength. ENDIF comments
 *    - transfer.c, perm_neg_check(), char * permdeny.
 *    - session.c, session(), char * abort.
 *    - directory.h, #includes, determine if there are ones that aren't required
 *    - help.c, command_HELP(), is the return value checked, should it be, or
 *      should the return be void.
 *****************************************************************************/

/******************************************************************************
 * Other TODO's:
 *
 *   - Return an error for when [net.c] send_all() fails. Handle this error.
 *   - Test/check the includes in "directory.h". errno.h, stdlib.h, stdio.h... 
 *   - Find errors which the server can recover from, currently most failures
 *     result in the server shutting down. Admittedly most of these errors are
 *     pretty fatal, but this should be looked into.
 *****************************************************************************/

/******************************************************************************
 * main
 *****************************************************************************/
int main (int argc, char *argv[])
{
  int listenSfd;        // Listen for control connections on this sfd.
  int *csfd;            // An accepted control socket file descriptor.
  pthread_t thread;     // The handle for a new thread.
  pthread_attr_t attr;  // pthread attribute, to set detached state on creation.
  
  char *rootTemp;

  //Retrieve the name of the root directory from the config file.
  if ((rootTemp = get_config_value ("ROOT_PATH_CONFIG", FTP_CONFIG_FILE)) == NULL)
    return -1;

  /* Append the relative path from the server executable to the server root directory
   * to the absolute path of the server executable. */
  if ((rootdir = get_config_path (rootTemp)) == NULL) {
    free (rootTemp);
    return -1;
  }
  free (rootTemp);

  rootTemp = rootdir;
  /* Canonicalize the path to the server root directory 
   * (eg. resolve all "..", ".", excessive "/" and symbolic links). */
  if ((rootdir = canonicalize_file_name (rootTemp)) == NULL) {
    fprintf (stderr, "%s: canonicalize_file_name: %s\n", __FUNCTION__, strerror (errno));
  }
  free (rootTemp);

  //Initialize the pthread attributes.
  if (pthread_attr_init (&attr) != 0) {
    fprintf (stderr, "%s: pthread_attr_init: %s\n", __FUNCTION__, strerror (errno));
    return -1;
  }

  //Set the detach state attribute.
  if (pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED) != 0) {
    fprintf (stderr, "%s: pthread_attr_init: %s\n", __FUNCTION__, strerror (errno));
    return -1;
  }

  //Create a socket to listen for control connections.
  if ((listenSfd = get_control_sock ()) == -1)
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
    if ((csfd = malloc (sizeof(*csfd))) == NULL) {
      fprintf (stderr, "%s: malloc of %lu bytes failed\n", __FUNCTION__, sizeof(*csfd));
      break;
    }

    //Accept a connection from the client, or read a server command on stdin.
    if ((*csfd = accept_connection (listenSfd, ACCEPT_CONTROL, NULL)) == -1) {
      free (csfd);
      continue;
    } else if (*csfd == STDIN_READY) {   //There is something to read on stdin.
      if (read_server_cmd () == SHUTDOWN_SERVER) {
	shutdownServer = true;
	free (csfd);
	break;
      } else {
	free (csfd);
	continue;
      }
    }

    //Create a new thread for this control connection.
    if (pthread_create (&thread, &attr, &control_thread, csfd) != 0) {
      fprintf (stderr, "%s: pthread_create: %s\n", __FUNCTION__, strerror (errno));
      free (csfd);
      continue;
    }

    //Increment the control connection thread count.
    if (modify_cthread_count (1) == -1)
      break;
  }

  free (rootdir);

  if (activeControlThreads > 0)
    printf ("waiting on threads to resolve...\n");

  //Wait for the control threads to shutdown.
  while (activeControlThreads > 0) {
    sleep (1);
  }

  if (pthread_attr_destroy (&attr) == -1)
    fprintf (stderr, "%s: pthread_attr_destroy: %s\n", __FUNCTION__, strerror (errno));

  printf ("All threads have terminated, exiting the program.\n");
  return 0;
}
