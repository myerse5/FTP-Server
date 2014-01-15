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


void cmd_user(session_info_t *si,char *arg) {
  
  //if user command is given, log current user out
  if (arg == NULL) {
    char *noarg = "501 Syntax error in parameters or arguments.\n";
    send_all(si->c_sfd, (uint8_t*)noarg, strlen(noarg));
    return;
  }
  si->logged_in = false;
  si->user[0] = '\0';
  
  //check if user is anonymous
  if (strcasecmp(arg,"anonymous") == 0) {
    si->logged_in = true;
    
    char *loggedin = "230 Login successful.\n";
    send_all(si->c_sfd, (uint8_t*)loggedin, strlen(loggedin));
    
  } else if (arg != NULL) {
    /* command will take any argument as a valid username in order
     * to prevent a malicious client from collecting usernames
     * from server.
     */
    char *needpass = "331 User name okay, need password.\n";
    send_all(si->c_sfd, (uint8_t*)needpass, strlen(needpass));
  }
  //as long as argument isn't null, copy the string over
  strcpy(si->user,arg);
  return;
}

void cmd_pass(session_info_t *si, char *cmd) {
  
  char *password = NULL;
  char *notfound = "530 Not logged in.\n";
  char md5string[33];
  
  //if user is logged in, no pass require
  if (si->logged_in) {
    char *loggedin = "230 Already logged in.\n";
    send_all(si->c_sfd, (uint8_t*)loggedin, strlen(loggedin));
    return;
  }
  
  
  //check if username has been given
  if (strlen(si->user) > 0) {
    if (cmd) {
      if ((password = get_config_value (si->user,USER_CONFIG_FILE)) == NULL) {
	send_all (si->c_sfd, (uint8_t*)notfound, strlen(notfound));
      } else {
	//get md5 of password + username
	getMD5(si->user,cmd,md5string);
	if (strcmp(md5string,password) == 0) {
	  char *loggedin = "230 Login successful.\n";
	  si->logged_in = true;
	  send_all(si->c_sfd, (uint8_t*)loggedin, strlen (loggedin));
	} else {
	  //found name but password didn't match
	  send_all(si->c_sfd, (uint8_t*)notfound, strlen (notfound));
	}
      }
    } else {
      //no argument was given
      char *syntaxerror = "501 Syntax error in arguments.\n";
      send_all(si->c_sfd, (uint8_t*)syntaxerror, strlen (syntaxerror));
      return;
    }
  } else {
    char *badsequence = "503 Login with USER first.\n";
    send_all(si->c_sfd, (uint8_t*)badsequence, strlen (badsequence));
  }
  
  if (password)
    free(password);
  
  return;
}

//TODO add function header
int getMD5 (char *user, char *password, char *md5string) {
  //TODO fix the buffer overruns in this block
  char data[500];
  byte_t md5checksum[16];
  strcpy (data,password);
  strcat (data,user);
  struct md5CTX md5struct;
  md5Start (&md5struct);
  md5Add (&md5struct,(byte_t*)data,strlen(data));
  md5End (&md5struct,md5checksum);
  for(int i = 0; i < 16; ++i)
    sprintf (&md5string[i*2], "%02x", (unsigned int)md5checksum[i]);
  return 0;
}
