/******************************************************************************
 * Authors: Evan Myers, Justin Slind, Alex Tai, James Yoo
 * FTP-Server
 * Date: November 2013
 *
 * Description:
 *   A massive 'if-else' statement that determines the command that has been
 *   invoked by the client and performs an appropriately related action.
 *****************************************************************************/
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "help.h"
#include "misc.h"
#include "transfer.h"
#include "parser.h"
#include "switch.h"
#include "directory.h"
#include "net.h"
#include "session.h"
#include "user.h"


//Preprocessor Macro Define(s)
#define MAX_CMD_SIZE 4    //Maximum allowed length of a command.
#define MIN_CMD_SIZE 3    //Minimum allowed length of a command.
#define MIN_NUM_ARGS 1    //Minimum allowed number of arguments.


/******************************************************************************
 * command_switch - see "switch.h"
 *****************************************************************************/
void *command_switch (void *param) {
  session_info_t *si;
  struct tm *timeInfo;
  time_t rawTime;
  int numArgs;
  char *arg, *cmd, *cmdLine;
  int csfd;

  char *cmdNotFound = "500 - Syntax error, command unrecognized.\n";
  char *cmdNotUsed = "502 - Command is not currently implemented.\n";

  si = (session_info_t *)param;
  cmdLine = si->cmdString;
  numArgs = get_arg_count (cmdLine);
  csfd = si->csfd;

  //The timeInfo is printed to the console for debugging/logging purposes.
  time (&rawTime);
  timeInfo = localtime (&rawTime);

  if (numArgs >= MIN_NUM_ARGS) {

    arg = separate_cmd_from_args (&cmdLine, numArgs);
    cmd = cmdLine;

    //cmd = extract_cmd_string (cmdLine);
    //arg = extract_arg_string (cmdLine);

    fprintf (stderr, "%s\tUser <%s>\n", asctime (timeInfo), si->user);
    fprintf (stderr, "\tInvoked Command <%s> with (%d) Argument(s) \"%s\"\n\n",
	     cmd, (numArgs - 1), arg);

    if (strlen (cmd) == MAX_CMD_SIZE) {
      //USER <SP> <username> <CRLF>
      if (strcmp (cmd, "USER") == 0) {
	cmd_user (si, arg);

	//PASS <SP> <password> <CRLF>
      } else if (strcmp (cmd, "PASS") == 0) {
	cmd_pass (si, arg);

	//QUIT <CRLF>
      } else if (strcmp (cmd, "QUIT") == 0) {
	cmd_quit (si);

	//PORT <SP> <host-port> <CRLF>
      } else if (strcmp (cmd, "PORT") == 0) {
	cmd_port (si, arg);

	//PASV <CRLF>
      } else if (strcmp (cmd, "PASV") == 0) {
	cmd_pasv (si);

	//TYPE <SP> <type-code> <CRLF>
      } else if (strcmp (cmd, "TYPE") == 0) {
	cmd_type (si, arg);

	//STRU <SP> <structure-code> <CRLF>
      } else if (strcmp (cmd, "STRU") == 0) {
	cmd_stru (si, arg, numArgs);

	//MODE <SP> <mode-code> <CRLF>
      } else if (strcmp (cmd, "MODE") == 0) {
    	cmd_mode (si, arg);

	//RETR <SP> <pathname> <CRLF>
      } else if (strcmp (cmd, "RETR") == 0) {
    	cmd_retr (si, arg);

	//STOR <SP> <pathname> <CRLF>
      } else if (strcmp (cmd, "STOR") == 0) {
    	cmd_stor (si, arg);

	//STOU <CRLF>
      } else if (strcmp (cmd, "STOU") == 0) {
	cmd_stou (si, arg);

	//APPE <SP> <pathname> <CRLF>
      } else if (strcmp (cmd, "APPE") == 0) {
	cmd_appe (si, arg);

	//LIST [<SP> <pathname>] <CRLF>
      } else if (strcmp (cmd, "LIST") == 0) {
	cmd_list_nlst (si, arg, true);

	//NLST [<SP> <pathname>] <CRLF>
      } else if (strcmp (cmd, "NLST") == 0) {
	cmd_list_nlst (si, arg, false);

	//SYST <CRLF>
      } else if (strcmp (cmd, "SYST") == 0) {
	cmd_syst (si);

	//HELP [<SP> <string>] <CRLF>
      } else if (strcmp (cmd, "HELP") == 0) {
	if (arg != NULL) {
	  convert_to_upper (arg);
	}
	cmd_help (si, arg);

	//NOOP <CRLF>
      } else if (strcmp (cmd, "NOOP") == 0) {
    	send_all (csfd, (uint8_t *)cmdNotUsed, strlen (cmdNotUsed));

	//ACCT <SP> <account-information> <CRLF>
      } else if (strcmp (cmd, "ACCT") == 0) {
    	send_all (csfd, (uint8_t *)cmdNotUsed, strlen (cmdNotUsed));

	//CDUP <CRLF>
      } else if (strcmp (cmd, "CDUP") == 0) {
	cmd_cdup (si, arg);

	//SMNT <SP> <pathname> <CRLF>
      } else if (strcmp (cmd, "SMNT") == 0) {
    	send_all (csfd, (uint8_t *)cmdNotUsed, strlen (cmdNotUsed));

	//REIN <CRLF>
      } else if (strcmp (cmd, "REIN") == 0) {
    	send_all (csfd, (uint8_t *)cmdNotUsed, strlen (cmdNotUsed));

	//ALLO <SP> <decimal-integer> [<SP> R <SP> <decimal-integer>] <CRLF>
      } else if (strcmp (cmd, "ALLO") == 0) {
    	send_all (csfd, (uint8_t *)cmdNotUsed, strlen (cmdNotUsed));

	//REST <SP> <marker> <CRLF>
      } else if (strcmp (cmd, "REST") == 0) {
    	send_all (csfd, (uint8_t *)cmdNotUsed, strlen (cmdNotUsed));

	//RNFR <SP> <pathname> <CRLF>
      } else if (strcmp (cmd, "RNFR") == 0) {
    	send_all (csfd, (uint8_t *)cmdNotUsed, strlen (cmdNotUsed));

	//RNTO <SP> <pathname> <CRLF>
      } else if (strcmp (cmd, "RNTO") == 0) {
    	send_all (csfd, (uint8_t *)cmdNotUsed, strlen (cmdNotUsed));

	//ABOR <CRLF>
      } else if (strcmp (cmd, "ABOR") == 0) {
    	send_all (csfd, (uint8_t *)cmdNotUsed, strlen (cmdNotUsed));

	//DELE <SP> <pathname> <CRLF>
      } else if (strcmp (cmd, "DELE") == 0) {
    	send_all (csfd, (uint8_t *)cmdNotUsed, strlen (cmdNotUsed));

	//SITE <SP> <string> <CRLF>
      } else if (strcmp (cmd, "SITE") == 0) {
    	send_all (csfd, (uint8_t *)cmdNotUsed, strlen (cmdNotUsed));

	//STAT [<SP> <pathname>] <CRLF>
      } else if (strcmp (cmd, "STAT") == 0) {
    	send_all (csfd, (uint8_t *)cmdNotUsed, strlen (cmdNotUsed));

      } else {
	fprintf (stderr, "%s\tUser <%s>\n", asctime (timeInfo), si->user);
	fprintf (stderr, "\tERROR: Command <%s> Unknown!\n", cmd);
	send_all (csfd, (uint8_t *)cmdNotFound, strlen (cmdNotFound));
      }

    } else if (strlen (cmd) == MIN_CMD_SIZE) {
      //CWD <SP> <pathname> <CRLF>
      if (strcmp (cmd, "CWD") == 0) {
	cmd_cwd (si, arg);

	//RMD <SP> <pathname> <CRLF>
      } else if (strcmp (cmd, "RMD") == 0) {
    	send_all (csfd, (uint8_t *)cmdNotUsed, strlen (cmdNotUsed));

	//MKD <SP> <pathname> <CRLF>
      } else if (strcmp (cmd, "MKD") == 0) {
	cmd_mkd (si, arg);

	//PWD <CRLF>
      } else if (strcmp (cmd, "PWD") == 0) {
	cmd_pwd (si);

      } else {
	send_all (csfd, (uint8_t *)cmdNotUsed, strlen (cmdNotUsed));
      }

    } else {
      fprintf (stderr, "%s\tUser <%s>\n", asctime (timeInfo), si->user);
      fprintf (stderr, "\tERROR: Command <%s> Unknown!\n", cmd);
      send_all (csfd, (uint8_t *)cmdNotFound, strlen (cmdNotFound));
    }

  } else {
    fprintf (stderr, "%s\tUser <%s>\n", asctime (timeInfo), si->user);
    fprintf (stderr, "\tERROR: Missing Command/Insufficient Arguments!\n");
  }

  si->cmdComplete = true;
  return NULL;
}