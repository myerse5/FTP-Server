/******************************************************************************
 * FTP-Server
 * Authors: Evan Myers, Justin Slind, Alex Tai, James Yoo
 * Date: November 2013
 *
 * Description:
 *   Functions to deal with users logging into the server.
 *****************************************************************************/
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <stdio.h>
#include "config.h"
#include "md5.h"
#include "net.h"
#include "reply.h"
#include "session.h"
#include "user.h"


//Local function prototypes.
static void get_md5 (char *user, char *password, char *md5str);


/******************************************************************************
 * cmd_user - see "users.h"
 *****************************************************************************/
void cmd_user (session_info_t *si, char *arg)
{
  int csfd = si->csfd;

  if (arg == NULL) {
    send_mesg_501 (csfd);
    return;
  }

  // If the USER command is given, log the current user out.
  si->loggedin = false;
  si->user[0] = '\0';
  
  // Check if user is anonymous. Anonymous user requires no password.
  if (strcasecmp (arg, "anonymous") == 0) {
    si->loggedin = true;
    send_mesg_230 (csfd, REPLY_230_SUCCESS);    
  } else if (arg != NULL) {
    /* Command will take any argument as a valid username in order
     * to prevent a malicious client from collecting usernames
     * from server. */
    send_mesg_331 (csfd);
  }

  // Copy the user into the session info.
  strcpy (si->user, arg);
  return;
}


/******************************************************************************
 * cmd_pass - see "users.h"
 *****************************************************************************/
void cmd_pass (session_info_t *si, char *arg)
{  
  char *pass = NULL;
  char md5string[33];
  int csfd = si->csfd;
 
  // If the client is logged in, no password is required.
  if (si->loggedin) {
    send_mesg_230 (csfd, REPLY_230_NONEED);
    return;
  }

  // Do not login if the client has not yet supplied a username.
  if (strlen (si->user) == 0) {
    send_mesg_503 (csfd);
    return;
  }

  // No argument was given, the PASS command fails.
  if (strlen (arg) == 0) {
    send_mesg_501 (csfd);
    return;
  }

  /* If no password has been set for the username entered by the client in the
   * previous USER command, abort the login. */
  if ((pass = get_config_value (si->user, USER_CONFIG_FILE)) == NULL) {
    send_mesg_530 (csfd, REPLY_530_FAIL);
    return;
  }

  // Get the MD5 of the password + username.
  get_md5 (si->user, arg, md5string);
  if (strcmp (md5string, pass) == 0) {
    send_mesg_230 (csfd, REPLY_230_SUCCESS);
    si->loggedin = true;
  } else {
    // The password did not match the password found in the user.conf file.
    send_mesg_530 (csfd, REPLY_530_FAIL);
  }
  
  free (pass);
  return;
}


/******************************************************************************
 * Convert a PASS argument to an MD5 to be compared with the password found in
 * "users.conf".
 *
 * Arguments:
 *   user - The currently logged in username.
 *   password - The argument given with the PASS command.
 *   md5str - The string to contain the md5 string.
 *****************************************************************************/
static void get_md5 (char *user, char *pass, char *md5str)
{
  /* TODO fix the buffer overruns in this block. Username could be greater than
   * 500 characters, password could be longer than 500 characters, both
   * combined may definitely be longer than 500 characters. */
  char data[500];
  byte_t md5checksum[16];
  struct md5CTX md5struct;

  //Salt the password with the username before creating the checksum.
  strcpy (data, pass);
  strcat (data, user);

  //Build the checksum.
  md5Start (&md5struct);
  md5Add (&md5struct, (byte_t*)data, strlen (data));
  md5End (&md5struct, md5checksum);

  //Create the checksum string.
  for(int i = 0; i < 16; ++i)
    sprintf (&md5str[i*2], "%02x", (unsigned int)md5checksum[i]);
}
