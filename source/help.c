/******************************************************************************
 * Authors: Evan Myers, Justin Slind, Alex Tai, James Yoo
 * FTP-Server
 * Date: November 2013
 *
 * Description:
 *   The help functions are found in this file.
 *****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "help.h"
#include "net.h"
#include "reply.h"
#include "session.h"


//Limit values for a legal command.
#define MAX_CMD_SIZE 4
#define MAX_NUM_ARGS 1
#define MIN_CMD_SIZE 3


/******************************************************************************
 * cmd_help - see "help.h"
 *****************************************************************************/
void cmd_help (session_info_t *si, char *arg)
{
  const char *notFound = "501 - Syntax error, command unrecognized.\n";
  int argLength;
  int csfd = si->csfd;

  //Determine if the client wants a list of commands or usage instructions.
  if (arg == NULL) {
    send_mesg_214_general (csfd);
    return;
  } else {
    argLength = strlen (arg);
  }


  if (argLength == MAX_CMD_SIZE) {
    /* USER <SP> <username> <CRLF> */
    if (strcmp (arg, "USER") == 0) {
      send_mesg_214_specific (csfd, "To use: USER <SP> <username> <CRLF>\n",
			     "\tSend new user information\n");

      /* PASS <SP> <password> <CRLF> */
    } else if (strcmp (arg, "PASS") == 0) {
      send_mesg_214_specific (csfd, "To use: PASS <SP> <password> <CRLF>\n",
			     "\tSend a user password\n");

      /* QUIT <CRLF> */
    } else if (strcmp (arg, "QUIT") == 0) {
      send_mesg_214_specific (csfd, "To use: QUIT <CRLF>\n",
			     "\tTerminate the FTP session and exit\n");

      /* PORT <SP> <host-port> <CRLF> */
    } else if (strcmp (arg, "PORT") == 0) {
      send_mesg_214_specific (csfd, "To use: PORT <SP> <host-port> <CRLF>\n",
			     "\tOpen a specified data port\n");

      /* PASV <CRLF> */
    } else if (strcmp (arg, "PASV") == 0) {
      send_mesg_214_specific (csfd, "To use: PASV <CRLF>\n",
			     "\tEnter passive transfer mode\n");

      /* TYPE <SP> <type-code> <CRLF> */
    } else if (strcmp (arg, "TYPE") == 0) {
      send_mesg_214_specific (csfd, "To use: TYPE <SP> <type-code> <CRLF>\n",
			     "\tSet file transfer type\n");

      /* STRU <SP> <structure-code> <CRLF> */
    } else if (strcmp (arg, "STRU") == 0) {
      send_mesg_214_specific (csfd, "To use: STRU <SP> <structure-code> <CRLF>\n",
			     "\tSet file transfer structure\n");

      /* MODE <SP> <mode-code> <CRLF> */
    } else if (strcmp (arg, "MODE") == 0) {
      send_mesg_214_specific (csfd, "To use: MODE <SP> <mode-code> <CRLF>\n",
			     "\tSet file transfer mode\n");

      /* RETR <SP> <pathname> <CRLF> */
    } else if (strcmp (arg, "RETR") == 0) {
      send_mesg_214_specific (csfd, "To use: RETR <SP> <pathname> <CRLF>\n",
			     "\tRetrieve a file from the remote host\n");

      /* STOR <SP> <pathname> <CRLF> */
    } else if (strcmp (arg, "STOR") == 0) {
      send_mesg_214_specific (csfd, "To use: STOR <SP> <pathname> <CRLF>\n",
			     "\tStore a file on the remote host\n");

      /* STOU <CRLF> */
    } else if (strcmp (arg, "STOU") == 0) {
      send_mesg_214_specific (csfd, "To use: STOU <CRLF>\n",
			     "\tStore a file uniquely\n");

      /* APPE <SP> <pathname> <CRLF> */
    } else if (strcmp (arg, "APPE") == 0) {
      send_mesg_214_specific (csfd, "To use: APPE <SP> <pathname> <CRLF>\n",
			     "\tAppend to a remote file\n");

      /* ABOR <CRLF> */
    } else if (strcmp (arg, "ABOR") == 0) {
      send_mesg_214_specific (csfd, "To use: ABOR <CRLF>\n",
			     "\tAbort a file transfer\n");

      /* LIST [<SP> <pathname>] <CRLF> */
    } else if (strcmp (arg, "LIST") == 0) {
      send_mesg_214_specific (csfd, "To use: LIST [<SP> <pathname>] <CRLF>\n",
			     "\tList contents of remote path\n");

      /* NLIST [<SP> <pathname>] <CRLF> */
    } else if (strcmp (arg, "NLST") == 0) {
      send_mesg_214_specific (csfd, "To use: NLST [<SP> <pathname>] <CRLF>\n",
			     "\tName list of remote directory\n");

      /* SYST <CRLF> */
    } else if (strcmp (arg, "SYST") == 0) {
      send_mesg_214_specific (csfd, "To use: SYST <CRLF>\n",
			     "\tReturn remote system type\n");

      /* HELP [<SP> <string>] <CRLF> */
    } else if (strcmp (arg, "HELP") == 0) {
      send_mesg_214_specific (csfd, "To use: HELP [<SP> <string>] <CRLF>\n",
			     "\tPrint local help information on using the server\n");

      /* NOOP <CRLF> */
    } else if (strcmp (arg, "NOOP") == 0) {
      send_mesg_214_specific (csfd, "To use: NOOP <CRLF>\n",
			     "\tDo nothing; server sends an OK reply\n");

      /* ACCT <SP> <account-information> <CRLF> */
    } else if (strcmp (arg, "ACCT") == 0) {
      send_mesg_214_specific (csfd, "To use: ACCT <SP> <account-information> <CRLF>\n",
			     "\tSend account command to the remote server\n");

      /* CDUP <CRLF> */
    } else if (strcmp (arg, "CDUP") == 0) {
      send_mesg_214_specific (csfd, "To use: CDUP <CRLF>\n",
			     "\tChange remote working directory to parent directory\n");

      /* SMNT <SP> <pathname> <CRLF> */
    } else if (strcmp (arg, "SMNT") == 0) {
      send_mesg_214_specific (csfd, "To use: SMNT <SP> <pathname> <CRLF>\n",
			     "\tMount a different file system data structure without"
			     "altering their login\n\tor accounting information\n");

      /* REIN <CRLF> */
    } else if (strcmp (arg, "REIN") == 0) {
      send_mesg_214_specific (csfd, "To use: REIN <CRLF>\n",
			     "\tReinitialize the connection\n");

      /* ALLO <SP> <decimal-integer> [<SP> R <SP> <decimal-integer>] <CRLF> */
    } else if (strcmp (arg, "ALLO") == 0) {
      send_mesg_214_specific (csfd, "To use: ALLO <SP> <decimal-integer> "
			     "[<SP> R <SP> <decimal-integer>] <CRLF>\n",
			     "\tAllocate sufficient storage space to receive a file\n");

      /* REST <SP> <marker> <CRLF> */
    } else if (strcmp (arg, "REST") == 0) {
      send_mesg_214_specific (csfd, "To use: REST <SP> <marker> <CRLF>\n",
			     "\tRestart transfer at bytecount marker\n");
      
      /* RNFR <SP> <pathname> <CRLF> */
    } else if (strcmp (arg, "RNFR") == 0) {
      send_mesg_214_specific (csfd, "To use: RNFR <SP> <pathname> <CRLF>\n",
			     "\tUsed when renaming a file; rename from\n");

      /* RNTO <SP> <pathname> <CRLF> */
    } else if (strcmp (arg, "RNTO") == 0) {
      send_mesg_214_specific (csfd, "To use: RNTO <SP> <pathname> <CRLF>\n",
			     "\tUsed when renaming a file; rename to\n");

      /* DELE <SP> <pathname> <CRLF> */
    } else if (strcmp (arg, "DELE") == 0) {
      send_mesg_214_specific (csfd, "To use: DELE <SP> <pathname> <CRLF>\n",
			     "\tDelete a remote file\n");

      /* SITE <SP> <string> <CRLF> */
    } else if (strcmp (arg, "SITE") == 0) {
      send_mesg_214_specific (csfd, "To use: SITE <SP> <string> <CRLF>\n",
			     "\tSend site specific command to the remote server\n");

      /* STAT [<SP> <pathname>] <CRLF> */
    } else if (strcmp (arg, "STAT") == 0) {
      send_mesg_214_specific (csfd, "To use: STAT [<SP> <pathname>] <CRLF>\n",
			     "\tReturn current server status\n");

    } else {
      send_all (csfd, (uint8_t *)notFound, strlen (notFound));
    }

  } else if (argLength == MIN_CMD_SIZE) {
      /* CWD <SP> <pathname> <CRLF> */
      if (strcmp (arg, "CWD") == 0) {
	send_mesg_214_specific (csfd, "To use: CWD <SP> <pathname> <CRLF>\n",
			       "\tChange the current working directory on the remote machine\n");

	/* RMD <SP> <pathname> <CRLF> */
      } else if (strcmp (arg, "RMD") == 0) {
	send_mesg_214_specific (csfd, "To use: RMD <SP> <pathname> <CRLF>\n",
			       "\tRemove a directory on the remote machine\n");

	/* MKD <SP> <pathname> <CRLF> */
      } else if (strcmp (arg, "MKD") == 0) {
	send_mesg_214_specific (csfd, "To use: MKD <SP> <pathname> <CRLF>\n",
			       "\tCreate a directory on the remote machine\n");

	/* PWD <CRLF> */
      } else if (strcmp (arg, "PWD") == 0) {
	send_mesg_214_specific (csfd, "To use: PWD <CRLF>\n",
			       "\tPrint the current working directory on the remote machine\n");

      } else {
	send_all (csfd, (uint8_t *)notFound, strlen (notFound));
      }
      
  } else {
    send_all (csfd, (uint8_t *)notFound, strlen (notFound));
  }
}
