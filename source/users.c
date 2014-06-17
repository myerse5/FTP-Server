/******************************************************************************
 * Students: Evan Myers, Justin Slind, Alex Tai, James Yoo
 * Course: CMPT-361
 * Assignment #3 - ftp server
 * File: users.c
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
#include "session.h"
#include "users.h"


//Local function prototypes.
static void getMD5 (char *user, char *password, char *md5str);


/******************************************************************************
 * cmd_user - see "users.h"
 *****************************************************************************/
void cmd_user (session_info_t *si, char *arg)
{
  char *reply;
  //if user command is given, log current user out
  if (arg == NULL) {
    reply = "501 Syntax error in parameters or arguments.\n";
    send_all (si->c_sfd, (uint8_t*)reply, strlen (reply));
    return;
  }
  si->logged_in = false;
  si->user[0] = '\0';
  
  //Check if user is anonymous. Anonymous user requires no password.
  if (strcasecmp (arg, "anonymous") == 0) {
    si->logged_in = true;
    
    reply = "230 Login successful.\n";
    send_all (si->c_sfd, (uint8_t*)reply, strlen (reply));
    
  } else if (arg != NULL) {
    /* Command will take any argument as a valid username in order
     * to prevent a malicious client from collecting usernames
     * from server.
     */
    reply = "331 User name okay, need password.\n";
    send_all (si->c_sfd, (uint8_t*)reply, strlen (reply));
  }
  //as long as argument isn't null, copy the string over
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
  char *reply;
 
  //if user is logged in, no pass require
  if (si->logged_in) {
    reply = "230 Already logged in.\n";
    send_all (si->c_sfd, (uint8_t*)reply, strlen (reply));
    return;
  }
  
  //Check if a username has been given.
  if (strlen(si->user) > 0) {
    if (arg) {
      if ((pass = get_config_value (si->user, USER_CONFIG_FILE)) == NULL) {
	reply = "530 Not logged in.\n";
	send_all (si->c_sfd, (uint8_t*)reply, strlen (reply));
      } else {
	//get md5 of password + username
	getMD5 (si->user, arg, md5string);
	if (strcmp (md5string, pass) == 0) {
	  reply = "230 Login successful.\n";
	  si->logged_in = true;
	  send_all (si->c_sfd, (uint8_t*)reply, strlen (reply));
	} else {
	  //The name was found but the password did not match.
	  reply = "530 Not logged in.\n";
	  send_all (si->c_sfd, (uint8_t*)reply, strlen (reply));
	}
      }
    } else {
      //No argument was given, the USER command fails.
      reply = "501 Syntax error in arguments.\n";
      send_all (si->c_sfd, (uint8_t*)reply, strlen (reply));
      return;
    }
  } else {
    reply = "503 Login with USER first.\n";
    send_all (si->c_sfd, (uint8_t*)reply, strlen (reply));
  }
  
  if (pass)
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
 *
 * Original author: Justin Slind
 * Updated by: Evan Myers
 *****************************************************************************/
static void getMD5 (char *user, char *pass, char *md5str)
{
  /* TODO fix the buffer overruns in this block. Username could be greater than
   * 500 characters, password could be longer than 500 characters, both
   * combined may definately be longer than 500 characters. */
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