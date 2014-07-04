/******************************************************************************
 * FTP-Server
 * Authors: Evan Myers, Justin Slind, Alex Tai, James Yoo
 * Date: November 2013
 *
 * Description:
 *   Miscellaneous ftp commands that don't fit in other categories at this
 *   time.
 *****************************************************************************/
#include <ctype.h>
#include <strings.h>
#include <string.h>
#include "misc.h"
#include "net.h"
#include "reply.h"
#include "session.h"


#define MAX_NUM_ARGS 1


/******************************************************************************
 * cmd_type - see "misc.h"
 *****************************************************************************/
void cmd_type (session_info_t *si, char *arg)
{
  int csfd = si->csfd;

  // The user must be logged in to change the type.
  if (!si->loggedin) {
    send_mesg_530 (csfd, REPLY_530_REQUEST);
    return;
  }

  if (!arg) {
    send_mesg_501 (csfd);
    return;
  }

  // If the arg is 1 char it must be an 'a' or 'i', case insensitive.
  if (strlen (arg) == 1) {
    arg[0] = tolower (arg[0]); // change arg to lowercase
    if (arg[0] == 'a') {
      send_mesg_200 (csfd, REPLY_200_ASCII);
      si->type = 'a';
    } else if (arg[0] == 'i') {
      send_mesg_200 (csfd, REPLY_200_IMAGE);
      si->type = 'i';
    } else {
      send_mesg_504 (csfd);
    }
    return;
  }

  if (strlen (arg) == 3) {
    // set interesting chars to lowercase
    if (strcasecmp (arg, "a n") == 0) {
      send_mesg_200 (csfd, REPLY_200_ASCII);
      si->type ='a';
      return;
    }
  }

  send_mesg_504 (csfd);
  return;
}


/******************************************************************************
 * cmd_mode - see "misc.h"
 *****************************************************************************/
void cmd_mode (session_info_t *si, char *arg)
{
  int csfd = si->csfd;

  if (arg) {
    if (strlen (arg) == 1) {
      arg[0] = tolower (arg[0]); // change arg to lowercase
      if (arg[0] == 's') {
	send_mesg_200 (csfd, REPLY_200_STREAM);
	return;	
      }
    }
  }

  send_mesg_504 (csfd);
  return;
}


/******************************************************************************
 * cmd_syst - see "misc.h"
 *****************************************************************************/
void cmd_syst (session_info_t *si)
{
  send_mesg_215 (si->csfd);
}


/******************************************************************************
 * cmd_stru - see "misc.h"
 *****************************************************************************/
void cmd_stru (session_info_t *si, char *arg, int numArgs)
{
  int csfd = si->csfd;

  if ((numArgs > MAX_NUM_ARGS) || (arg == NULL)) {
    send_mesg_501 (csfd);
    return;
  }

  if (strcmp (arg, "F") == 0) {
    send_mesg_200 (csfd, REPLY_200_FSTRU);
  } else {
    send_mesg_504 (csfd);
  }
}


/******************************************************************************
 * cmd_quit - see "cmd_quit.h"
 *****************************************************************************/
void cmd_quit (session_info_t *si)
{
  send_mesg_221 (si->csfd);
  si->cmdQuit = true;
}
