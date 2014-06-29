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
#include "users.h"


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
  int argCount, numArgs;
  char *arg, *cmd, *cmdLine;

  char *cmdUnrecognized = "500 - Syntax error, command unrecognized.\n",
       *cmdUnimplemented = "502 - Command is not currently implemented.\n";

  si = (session_info_t *)param;
  cmdLine = si->cmdString;
  numArgs = command_arg_count (cmdLine);
  argCount = 0;

  //The timeInfo is printed to the console for debugging/logging purposes.
  time (&rawTime);
  timeInfo = localtime (&rawTime);

  if (numArgs >= MIN_NUM_ARGS) {

    cmd = command_extract_cmd (cmdLine);
    arg = command_extract_arg (cmdLine);

    fprintf(stderr, "%s\tUser <%s>\n\tInvoked Command <%s> with (%d) Argument(s) \"%s\"\n\n", asctime(timeInfo), si->user, cmd, (numArgs - 1), arg);

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
	cmd_stru (si, arg, argCount);

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

	command_help (si, arg);

	//NOOP <CRLF>
      } else if (strcmp (cmd, "NOOP") == 0) {
    	send_all (si->csfd, (uint8_t *)cmdUnimplemented, strlen (cmdUnimplemented));

	//ACCT <SP> <account-information> <CRLF>
      } else if (strcmp (cmd, "ACCT") == 0) {
    	send_all (si->csfd, (uint8_t *)cmdUnimplemented, strlen (cmdUnimplemented));

	//CDUP <CRLF>
      } else if (strcmp (cmd, "CDUP") == 0) {
	cmd_cdup (si, arg);

	//SMNT <SP> <pathname> <CRLF>
      } else if (strcmp (cmd, "SMNT") == 0) {
    	send_all (si->csfd, (uint8_t *)cmdUnimplemented, strlen (cmdUnimplemented));

	//REIN <CRLF>
      } else if (strcmp (cmd, "REIN") == 0) {
    	send_all (si->csfd, (uint8_t *)cmdUnimplemented, strlen (cmdUnimplemented));

	//ALLO <SP> <decimal-integer> [<SP> R <SP> <decimal-integer>] <CRLF>
      } else if (strcmp (cmd, "ALLO") == 0) {
    	send_all (si->csfd, (uint8_t *)cmdUnimplemented, strlen (cmdUnimplemented));

	//REST <SP> <marker> <CRLF>
      } else if (strcmp (cmd, "REST") == 0) {
    	send_all (si->csfd, (uint8_t *)cmdUnimplemented, strlen (cmdUnimplemented));

	//RNFR <SP> <pathname> <CRLF>
      } else if (strcmp (cmd, "RNFR") == 0) {
    	send_all (si->csfd, (uint8_t *)cmdUnimplemented, strlen (cmdUnimplemented));

	//RNTO <SP> <pathname> <CRLF>
      } else if (strcmp (cmd, "RNTO") == 0) {
    	send_all (si->csfd, (uint8_t *)cmdUnimplemented, strlen (cmdUnimplemented));

	//ABOR <CRLF>
      } else if (strcmp (cmd, "ABOR") == 0) {
    	send_all (si->csfd, (uint8_t *)cmdUnimplemented, strlen (cmdUnimplemented));

	//DELE <SP> <pathname> <CRLF>
      } else if (strcmp (cmd, "DELE") == 0) {
    	send_all (si->csfd, (uint8_t *)cmdUnimplemented, strlen (cmdUnimplemented));

	//SITE <SP> <string> <CRLF>
      } else if (strcmp (cmd, "SITE") == 0) {
    	send_all (si->csfd, (uint8_t *)cmdUnimplemented, strlen (cmdUnimplemented));

	//STAT [<SP> <pathname>] <CRLF>
      } else if (strcmp (cmd, "STAT") == 0) {
    	send_all (si->csfd, (uint8_t *)cmdUnimplemented, strlen (cmdUnimplemented));

      } else {
	fprintf (stderr, "%s\tUser <%s>\n\tERROR: Command <%s> Unknown!\n", asctime (timeInfo), si->user, cmd);
	send_all (si->csfd, (uint8_t *)cmdUnimplemented, strlen (cmdUnimplemented));
      }

    } else if (strlen (cmd) == MIN_CMD_SIZE) {
      //CWD <SP> <pathname> <CRLF>
      if (strcmp (cmd, "CWD") == 0) {
	cmd_cwd (si, arg);

	//RMD <SP> <pathname> <CRLF>
      } else if (strcmp (cmd, "RMD") == 0) {
    	send_all (si->csfd, (uint8_t *)cmdUnimplemented, strlen (cmdUnimplemented));

	//MKD <SP> <pathname> <CRLF>
      } else if (strcmp (cmd, "MKD") == 0) {
	cmd_mkd (si, arg);

	//PWD <CRLF>
      } else if (strcmp (cmd, "PWD") == 0) {
	cmd_pwd (si);

      } else {
	send_all (si->csfd, (uint8_t *)cmdUnimplemented, strlen (cmdUnimplemented));
      }

    } else {
      fprintf (stderr, "%s\tUser <%s>\n\tERROR: Command <%s> Unknown!\n", asctime (timeInfo), si->user, cmd);
      send_all (si->csfd, (uint8_t *)cmdUnrecognized, strlen (cmdUnrecognized));
    }

    free (cmd);
    free (arg);

  } else {
    fprintf (stderr, "%s\tUser <%s>\n\tERROR: Missing Command/Insufficient Arguments!\n", asctime (timeInfo), si->user);
  }

  si->cmdComplete = true;
  return NULL;
}
