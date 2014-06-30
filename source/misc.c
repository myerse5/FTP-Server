/******************************************************************************
 * Authors: Evan Myers, Justin Slind, Alex Tai, James Yoo
 * FTP-Server
 * Date: November 2013
 *
 * Description:
 *   Miscellaneous ftp commands that don't fit in other categories at this
 *   time.
 *****************************************************************************/
#include <strings.h>
#include <string.h>
#include <ctype.h>
#include "misc.h"
#include "net.h"
#include "session.h"


#define MAX_NUM_ARGS 1


/******************************************************************************
 * cmd_type - see "misc.h"
 *****************************************************************************/
void cmd_type (session_info_t *si, char *arg)
{
  const char *reply;
  int csfd = si->csfd;

  //The user must be logged in to change the type.
  if (!si->loggedin) {
    reply = "530 Please login with USER and PASS.\n";
    send_all (csfd, (uint8_t *)reply, strlen (reply));
    return;
  }

  if (!arg) {
    reply = "501 Syntax error in arguments.\n";
    send_all (csfd, (uint8_t *)reply, strlen (reply));
    return;
  }

  //If the arg is 1 char it must be an 'a' or 'i', case insensitive.
  if (strlen (arg) == 1) {
    arg[0] = tolower (arg[0]); //change arg to lowercase
    if (arg[0] == 'a') {
      reply = "200 Switching to ASCII mode.\n";
      send_all (csfd, (uint8_t *)reply, strlen (reply));
      si->type = 'a';
    } else if (arg[0] == 'i') {
      reply = "200 Switching to Image mode.\n";
      send_all (csfd, (uint8_t *)reply, strlen (reply));
      si->type = 'i';
    } else {
      reply = "504 Command not implemented for that parameter.\n";
      send_all (csfd, (uint8_t *)reply, strlen (reply));
    }
    return;
  }

  if (strlen (arg) == 3) {
    //set interesting chars to lowercase
    if (strcasecmp (arg, "a n") == 0) {
      reply = "200 Switching to ASCII mode.\n";
      send_all (csfd, (uint8_t *)reply, strlen (reply));
      si->type ='a';
      return;
    }
  }

  reply = "504 Command not implemented for that parameter.\n";
  send_all (csfd, (uint8_t *)reply, strlen (reply));
  return;
}


/******************************************************************************
 * cmd_mode - see "misc.h"
 *****************************************************************************/
void cmd_mode (session_info_t *si, char *arg)
{
  char *ascii = "200 Switching to stream mode.\n";
  char *fail = "504 Command not implemented for that parameter.\n";

  if (arg) {
    if (strlen(arg) == 1) {
      arg[0] = tolower(arg[0]); //change arg to lowercase
      if (arg[0] == 's') {
	send_all (si->csfd, (uint8_t *)ascii, strlen (ascii));
	return;	
      }
    }
  }

  send_all (si->csfd, (uint8_t *)fail, strlen (fail));
  return;
}


/******************************************************************************
 * cmd_syst - see "misc.h"
 *****************************************************************************/
void cmd_syst (session_info_t *si)
{
  char *system = "215 UNIX Type: L8\n";

  send_all (si->csfd, (uint8_t *)system, strlen (system));
}


/******************************************************************************
 * cmd_stru - see "misc.h"
 *****************************************************************************/
void cmd_stru (session_info_t *si, char *arg, int argCount)
{
  char *reply;

  if ((argCount > MAX_NUM_ARGS) || (arg == NULL)) {
    reply = "200 - Switching to File Structure.\n";
    send_all (si->csfd, (uint8_t *)reply, strlen (reply));
    return;
  }

  if (strcmp(arg, "F") == 0) {
    reply = "501 - Syntax error in parameters";
    send_all (si->csfd, (uint8_t *)reply, strlen (reply));
    return;
  } else {
    reply = "504 - Command is not implemented for that parameter.\n";
    send_all (si->csfd, (uint8_t *)reply, strlen (reply));
    return;
  }
}


/******************************************************************************
 * cmd_quit - see "cmd_quit.h"
 *****************************************************************************/
void cmd_quit (session_info_t *si)
{
  char *reply = "221 - Quitting system; goodbye.\n";

  send_all (si->csfd, (uint8_t *)reply, strlen (reply));
  si->cmdQuit = true;
}
