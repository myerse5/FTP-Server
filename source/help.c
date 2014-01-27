/******************************************************************************
 * Students: Evan Myers, Justin Slind, Alex Tai, James Yoo
 * Course: CMPT-361
 * Assignment #3 - ftp server
 * File: help.c
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
#include "session.h"


#define CMD_STRING_LENGTH 128
#define MAX_CMD_SIZE 4
#define MAX_NUM_ARGS 1
#define MIN_CMD_SIZE 3


/******************************************************************************
 * command_help - see "help.h"
 *****************************************************************************/
int command_help (session_info_t *si, char *arg)
{ //BEGIN function 'command_help'

  const char *helpMsgEnd = "214 - Help is OK.\n\n",
             *helpMsgStart = "\n214 - Help message.\n";

  if (arg == NULL) {

    const char *row1 = "The following commands may be abbreviated\nThe available commands are:\n",
               *row2 = "\tABOR\tHELP\tPASV\tRMD\tSTOU\n",
               *row3 = "\tACCT\tLIST\tPORT\tRNFR\tSTRU\n",
               *row4 = "\tALLO\tMKD\tPWD\tRNTO\tSYST\n",
               *row5 = "\tAPPE\tMODE\tQUIT\tSITE\tTYPE\n",
               *row6 = "\tCDUP\tNLST\tREIN\tSMNT\tUSER\n",
               *row7 = "\tCWD\tNOOP\tREST\tSTAT\n",
               *row8 = "\tDELE\tPASS\tRETR\tSTOR\n";

    send_all(si->c_sfd, (uint8_t *)helpMsgStart, strlen(helpMsgStart));
    send_all(si->c_sfd, (uint8_t *)row1, strlen(row1));
    send_all(si->c_sfd, (uint8_t *)row2, strlen(row2));
    send_all(si->c_sfd, (uint8_t *)row3, strlen(row3));
    send_all(si->c_sfd, (uint8_t *)row4, strlen(row4));
    send_all(si->c_sfd, (uint8_t *)row5, strlen(row5));
    send_all(si->c_sfd, (uint8_t *)row6, strlen(row6));
    send_all(si->c_sfd, (uint8_t *)row7, strlen(row7));
    send_all(si->c_sfd, (uint8_t *)row8, strlen(row8));
    send_all(si->c_sfd, (uint8_t *)helpMsgEnd, strlen(helpMsgEnd));

    return 0;

  } else {

    int strLength;

    char *cmdInfo,
         *cmdLine;

    const char *argUnrecognized = "501 - Syntax error, command unrecognized.\n";

    if ((cmdLine = (char *)calloc((CMD_STRING_LENGTH + 1), sizeof(*cmdLine))) == NULL) {
      return -1;
    } //END statement 'if'

    if ((cmdInfo = (char *)calloc((CMD_STRING_LENGTH + 1), sizeof(*cmdInfo))) == NULL) {
      return -1;
    } //END statement 'if'

    if (strlen(arg) == MAX_CMD_SIZE) {

      /* USER <SP> <username> <CRLF> */
      if (strcmp(arg, "USER") == 0) {

	strLength = snprintf(cmdLine, (CMD_STRING_LENGTH + 1), "To use: %s <SP> <username> <CRLF>\n", arg);
	strncpy(cmdInfo, "\tSend new user information\n", (CMD_STRING_LENGTH + 1));

	send_all(si->c_sfd, (uint8_t *)helpMsgStart, strlen(helpMsgStart));
	send_all(si->c_sfd, (uint8_t *)cmdLine, strLength);
	send_all(si->c_sfd, (uint8_t *)cmdInfo, strlen(cmdInfo));
	send_all(si->c_sfd, (uint8_t *)helpMsgEnd, strlen(helpMsgEnd));

	/* PASS <SP> <password> <CRLF> */
      } else if (strcmp(arg, "PASS") == 0) {

	strLength = snprintf(cmdLine, (CMD_STRING_LENGTH + 1), "To use: %s <SP> <password> <CRLF>\n", arg);
	strncpy(cmdInfo, "\tSend a user password\n", (CMD_STRING_LENGTH + 1));

	send_all(si->c_sfd, (uint8_t *)helpMsgStart, strlen(helpMsgStart));
	send_all(si->c_sfd, (uint8_t *)cmdLine, strLength);
	send_all(si->c_sfd, (uint8_t *)cmdInfo, strlen(cmdInfo));
	send_all(si->c_sfd, (uint8_t *)helpMsgEnd, strlen(helpMsgEnd));

	/* QUIT <CRLF> */
      } else if (strcmp(arg, "QUIT") == 0) {

	strLength = snprintf(cmdLine, (CMD_STRING_LENGTH + 1), "To use: %s <CRLF>\n", arg);
	strncpy(cmdInfo, "\tTerminate the FTP session and exit\n", (CMD_STRING_LENGTH + 1));

	send_all(si->c_sfd, (uint8_t *)helpMsgStart, strlen(helpMsgStart));
	send_all(si->c_sfd, (uint8_t *)cmdLine, strLength);
	send_all(si->c_sfd, (uint8_t *)cmdInfo, strlen(cmdInfo));
	send_all(si->c_sfd, (uint8_t *)helpMsgEnd, strlen(helpMsgEnd));

	/* PORT <SP> <host-port> <CRLF> */
      } else if (strcmp(arg, "PORT") == 0) {

	strLength = snprintf(cmdLine, (CMD_STRING_LENGTH + 1), "To use: %s <SP> <host-port> <CRLF>\n", arg);
	strncpy(cmdInfo, "\tOpen a specified data port\n", (CMD_STRING_LENGTH + 1));

	send_all(si->c_sfd, (uint8_t *)helpMsgStart, strlen(helpMsgStart));
	send_all(si->c_sfd, (uint8_t *)cmdLine, strLength);
	send_all(si->c_sfd, (uint8_t *)cmdInfo, strlen(cmdInfo));
	send_all(si->c_sfd, (uint8_t *)helpMsgEnd, strlen(helpMsgEnd));

	/* PASV <CRLF> */
      } else if (strcmp(arg, "PASV") == 0) {

	strLength = snprintf(cmdLine, (CMD_STRING_LENGTH + 1), "To use: %s <CRLF>\n", arg);
	strncpy(cmdInfo, "\tEnter passive transfer mode\n", (CMD_STRING_LENGTH + 1));

	send_all(si->c_sfd, (uint8_t *)helpMsgStart, strlen(helpMsgStart));
	send_all(si->c_sfd, (uint8_t *)cmdLine, strLength);
	send_all(si->c_sfd, (uint8_t *)cmdInfo, strlen(cmdInfo));
	send_all(si->c_sfd, (uint8_t *)helpMsgEnd, strlen(helpMsgEnd));

	/* TYPE <SP> <type-code> <CRLF> */
      } else if (strcmp(arg, "TYPE") == 0) {

	strLength = snprintf(cmdLine, (CMD_STRING_LENGTH + 1), "To use: %s <SP> <type-code> <CRLF>\n", arg);
	strncpy(cmdInfo, "\tSet file transfer type\n", (CMD_STRING_LENGTH + 1));

	send_all(si->c_sfd, (uint8_t *)helpMsgStart, strlen(helpMsgStart));
	send_all(si->c_sfd, (uint8_t *)cmdLine, strLength);
	send_all(si->c_sfd, (uint8_t *)cmdInfo, strlen(cmdInfo));
	send_all(si->c_sfd, (uint8_t *)helpMsgEnd, strlen(helpMsgEnd));

	/* STRU <SP> <structure-code> <CRLF> */
      } else if (strcmp(arg, "STRU") == 0) {

	strLength = snprintf(cmdLine, (CMD_STRING_LENGTH + 1), "To use: %s <SP> <structure-code> <CRLF>\n", arg);
	strncpy(cmdInfo, "\tSet file transfer structure\n", (CMD_STRING_LENGTH + 1));

	send_all(si->c_sfd, (uint8_t *)helpMsgStart, strlen(helpMsgStart));
	send_all(si->c_sfd, (uint8_t *)cmdLine, strLength);
	send_all(si->c_sfd, (uint8_t *)cmdInfo, strlen(cmdInfo));
	send_all(si->c_sfd, (uint8_t *)helpMsgEnd, strlen(helpMsgEnd));

	/* MODE <SP> <mode-code> <CRLF> */
      } else if (strcmp(arg, "MODE") == 0) {

	strLength = snprintf(cmdLine, (CMD_STRING_LENGTH + 1), "To use: %s <SP> <mode-code> <CRLF>\n", arg);
	strncpy(cmdInfo, "\tSet file transfer mode\n", (CMD_STRING_LENGTH + 1));

	send_all(si->c_sfd, (uint8_t *)helpMsgStart, strlen(helpMsgStart));
	send_all(si->c_sfd, (uint8_t *)cmdLine, strLength);
	send_all(si->c_sfd, (uint8_t *)cmdInfo, strlen(cmdInfo));
	send_all(si->c_sfd, (uint8_t *)helpMsgEnd, strlen(helpMsgEnd));

	/* RETR <SP> <pathname> <CRLF> */
      } else if (strcmp(arg, "RETR") == 0) {

	strLength = snprintf(cmdLine, (CMD_STRING_LENGTH + 1), "To use: %s <SP> <pathname> <CRLF>\n", arg);
	strncpy(cmdInfo, "\tRetrieve a file from the remote host\n", (CMD_STRING_LENGTH + 1));

	send_all(si->c_sfd, (uint8_t *)helpMsgStart, strlen(helpMsgStart));
	send_all(si->c_sfd, (uint8_t *)cmdLine, strLength);
	send_all(si->c_sfd, (uint8_t *)cmdInfo, strlen(cmdInfo));
	send_all(si->c_sfd, (uint8_t *)helpMsgEnd, strlen(helpMsgEnd));

	/* STOR <SP> <pathname> <CRLF> */
      } else if (strcmp(arg, "STOR") == 0) {

	strLength = snprintf(cmdLine, (CMD_STRING_LENGTH + 1), "To use: %s <SP> <pathname> <CRLF>\n", arg);
	strncpy(cmdInfo, "\tStore a file on the remote host\n", (CMD_STRING_LENGTH + 1));

	send_all(si->c_sfd, (uint8_t *)helpMsgStart, strlen(helpMsgStart));
	send_all(si->c_sfd, (uint8_t *)cmdLine, strLength);
	send_all(si->c_sfd, (uint8_t *)cmdInfo, strlen(cmdInfo));
	send_all(si->c_sfd, (uint8_t *)helpMsgEnd, strlen(helpMsgEnd));

	/* STOU <CRLF> */
      } else if (strcmp(arg, "STOU") == 0) {

	strLength = snprintf(cmdLine, (CMD_STRING_LENGTH + 1), "To use: %s <CRLF>\n", arg);
	strncpy(cmdInfo, "\tStore a file uniquely\n", (CMD_STRING_LENGTH + 1));

	send_all(si->c_sfd, (uint8_t *)helpMsgStart, strlen(helpMsgStart));
	send_all(si->c_sfd, (uint8_t *)cmdLine, strLength);
	send_all(si->c_sfd, (uint8_t *)cmdInfo, strlen(cmdInfo));
	send_all(si->c_sfd, (uint8_t *)helpMsgEnd, strlen(helpMsgEnd));

	/* APPE <SP> <pathname> <CRLF> */
      } else if (strcmp(arg, "APPE") == 0) {

	strLength = snprintf(cmdLine, (CMD_STRING_LENGTH + 1), "To use: %s <SP> <pathname> <CRLF>\n", arg);
	strncpy(cmdInfo, "\tAppend to a remote file\n", (CMD_STRING_LENGTH + 1));

	send_all(si->c_sfd, (uint8_t *)helpMsgStart, strlen(helpMsgStart));
	send_all(si->c_sfd, (uint8_t *)cmdLine, strLength);
	send_all(si->c_sfd, (uint8_t *)cmdInfo, strlen(cmdInfo));
	send_all(si->c_sfd, (uint8_t *)helpMsgEnd, strlen(helpMsgEnd));

	/* ABOR <CRLF> */
      } else if (strcmp(arg, "ABOR") == 0) {

	strLength = snprintf(cmdLine, (CMD_STRING_LENGTH + 1), "To use: %s <CRLF>\n", arg);
	strncpy(cmdInfo, "\tAbort a file transfer\n", (CMD_STRING_LENGTH + 1));

	send_all(si->c_sfd, (uint8_t *)helpMsgStart, strlen(helpMsgStart));
	send_all(si->c_sfd, (uint8_t *)cmdLine, strLength);
	send_all(si->c_sfd, (uint8_t *)cmdInfo, strlen(cmdInfo));
	send_all(si->c_sfd, (uint8_t *)helpMsgEnd, strlen(helpMsgEnd));

	/* LIST [<SP> <pathname>] <CRLF> */
      } else if (strcmp(arg, "LIST") == 0) {

	strLength = snprintf(cmdLine, (CMD_STRING_LENGTH + 1), "To use: %s [<SP> <pathname>] <CRLF>\n", arg);
	strncpy(cmdInfo, "\tList contents of remote path\n", (CMD_STRING_LENGTH + 1));

	send_all(si->c_sfd, (uint8_t *)helpMsgStart, strlen(helpMsgStart));
	send_all(si->c_sfd, (uint8_t *)cmdLine, strLength);
	send_all(si->c_sfd, (uint8_t *)cmdInfo, strlen(cmdInfo));
	send_all(si->c_sfd, (uint8_t *)helpMsgEnd, strlen(helpMsgEnd));

	/* NLIST [<SP> <pathname>] <CRLF> */
      } else if (strcmp(arg, "NLST") == 0) {

	strLength = snprintf(cmdLine, (CMD_STRING_LENGTH + 1), "To use: %s [<SP> <pathname>] <CRLF>\n", arg);
	strncpy(cmdInfo, "\tName list of remote directory\n", (CMD_STRING_LENGTH + 1));

	send_all(si->c_sfd, (uint8_t *)helpMsgStart, strlen(helpMsgStart));
	send_all(si->c_sfd, (uint8_t *)cmdLine, strLength);
	send_all(si->c_sfd, (uint8_t *)cmdInfo, strlen(cmdInfo));
	send_all(si->c_sfd, (uint8_t *)helpMsgEnd, strlen(helpMsgEnd));

	/* SYST <CRLF> */
      } else if (strcmp(arg, "SYST") == 0) {

	strLength = snprintf(cmdLine, (CMD_STRING_LENGTH + 1), "To use: %s <CRLF>\n", arg);
	strncpy(cmdInfo, "\tReturn remote system type\n", (CMD_STRING_LENGTH + 1));

	send_all(si->c_sfd, (uint8_t *)helpMsgStart, strlen(helpMsgStart));
	send_all(si->c_sfd, (uint8_t *)cmdLine, strLength);
	send_all(si->c_sfd, (uint8_t *)cmdInfo, strlen(cmdInfo));
	send_all(si->c_sfd, (uint8_t *)helpMsgEnd, strlen(helpMsgEnd));

	/* HELP [<SP> <string>] <CRLF> */
      } else if (strcmp(arg, "HELP") == 0) {

	strLength = snprintf(cmdLine, (CMD_STRING_LENGTH + 1), "To use: %s [<SP> <string>] <CRLF>\n", arg);
	strncpy(cmdInfo, "\tPrint local help information on using the server\n", (CMD_STRING_LENGTH + 1));

	send_all(si->c_sfd, (uint8_t *)helpMsgStart, strlen(helpMsgStart));
	send_all(si->c_sfd, (uint8_t *)cmdLine, strLength);
	send_all(si->c_sfd, (uint8_t *)cmdInfo, strlen(cmdInfo));
	send_all(si->c_sfd, (uint8_t *)helpMsgEnd, strlen(helpMsgEnd));

	/* NOOP <CRLF> */
      } else if (strcmp(arg, "NOOP") == 0) {

	strLength = snprintf(cmdLine, (CMD_STRING_LENGTH + 1), "To use: %s <CRLF>\n", arg);
	strncpy(cmdInfo, "\tDo nothing; server sends an OK reply\n", (CMD_STRING_LENGTH + 1));

	send_all(si->c_sfd, (uint8_t *)helpMsgStart, strlen(helpMsgStart));
	send_all(si->c_sfd, (uint8_t *)cmdLine, strLength);
	send_all(si->c_sfd, (uint8_t *)cmdInfo, strlen(cmdInfo));
	send_all(si->c_sfd, (uint8_t *)helpMsgEnd, strlen(helpMsgEnd));

	/* ACCT <SP> <account-information> <CRLF> */
      } else if (strcmp(arg, "ACCT") == 0) {

	strLength = snprintf(cmdLine, (CMD_STRING_LENGTH + 1), "To use: %s <SP> <account-information> <CRLF>\n", arg);
	strncpy(cmdInfo, "\tSend account command to the remote server\n", (CMD_STRING_LENGTH + 1));

	send_all(si->c_sfd, (uint8_t *)helpMsgStart, strlen(helpMsgStart));
	send_all(si->c_sfd, (uint8_t *)cmdLine, strLength);
	send_all(si->c_sfd, (uint8_t *)cmdInfo, strlen(cmdInfo));
	send_all(si->c_sfd, (uint8_t *)helpMsgEnd, strlen(helpMsgEnd));

	/* CDUP <CRLF> */
      } else if (strcmp(arg, "CDUP") == 0) {

	strLength = snprintf(cmdLine, (CMD_STRING_LENGTH + 1), "To use: %s <CRLF>\n", arg);
	strncpy(cmdInfo, "\tChange remote working directory to parent directory\n", (CMD_STRING_LENGTH + 1));

	send_all(si->c_sfd, (uint8_t *)helpMsgStart, strlen(helpMsgStart));
	send_all(si->c_sfd, (uint8_t *)cmdLine, strLength);
	send_all(si->c_sfd, (uint8_t *)cmdInfo, strlen(cmdInfo));
	send_all(si->c_sfd, (uint8_t *)helpMsgEnd, strlen(helpMsgEnd));

	/* SMNT <SP> <pathname> <CRLF> */
      } else if (strcmp(arg, "SMNT") == 0) {

	strLength = snprintf(cmdLine, (CMD_STRING_LENGTH + 1), "To use: %s <SP> <pathname> <CRLF>\n", arg);
	strncpy(cmdInfo, "\tMount a different file system data structure without altering their login or accounting information\n", (CMD_STRING_LENGTH + 1));

	send_all(si->c_sfd, (uint8_t *)helpMsgStart, strlen(helpMsgStart));
	send_all(si->c_sfd, (uint8_t *)cmdLine, strLength);
	send_all(si->c_sfd, (uint8_t *)cmdInfo, strlen(cmdInfo));
	send_all(si->c_sfd, (uint8_t *)helpMsgEnd, strlen(helpMsgEnd));

	/* REIN <CRLF> */
      } else if (strcmp(arg, "REIN") == 0) {

	strLength = snprintf(cmdLine, (CMD_STRING_LENGTH + 1), "To use: %s <CRLF>\n", arg);
	strncpy(cmdInfo, "\tReinitialize the connection\n", (CMD_STRING_LENGTH + 1));

	send_all(si->c_sfd, (uint8_t *)helpMsgStart, strlen(helpMsgStart));
	send_all(si->c_sfd, (uint8_t *)cmdLine, strLength);
	send_all(si->c_sfd, (uint8_t *)cmdInfo, strlen(cmdInfo));
	send_all(si->c_sfd, (uint8_t *)helpMsgEnd, strlen(helpMsgEnd));

	/* ALLO <SP> <decimal-integer> [<SP> R <SP> <decimal-integer>] <CRLF> */
      } else if (strcmp(arg, "ALLO") == 0) {

	strLength = snprintf(cmdLine, (CMD_STRING_LENGTH + 1), "To use: %s <SP> <decimal-integer> [<SP> R <SP> <decimal-integer>] <CRLF>\n", arg);
	strncpy(cmdInfo, "\tAllocate sufficient storage space to receive a file\n", (CMD_STRING_LENGTH + 1));

	send_all(si->c_sfd, (uint8_t *)helpMsgStart, strlen(helpMsgStart));
	send_all(si->c_sfd, (uint8_t *)cmdLine, strLength);
	send_all(si->c_sfd, (uint8_t *)cmdInfo, strlen(cmdInfo));
	send_all(si->c_sfd, (uint8_t *)helpMsgEnd, strlen(helpMsgEnd));

	/* REST <SP> <marker> <CRLF> */
      } else if (strcmp(arg, "REST") == 0) {

	strLength = snprintf(cmdLine, (CMD_STRING_LENGTH + 1), "To use: %s <SP> <marker> <CRLF>\n", arg);
	strncpy(cmdInfo, "\tRestart transfer at bytecount marker\n", (CMD_STRING_LENGTH + 1));

	send_all(si->c_sfd, (uint8_t *)helpMsgStart, strlen(helpMsgStart));
	send_all(si->c_sfd, (uint8_t *)cmdLine, strLength);
	send_all(si->c_sfd, (uint8_t *)cmdInfo, strlen(cmdInfo));
	send_all(si->c_sfd, (uint8_t *)helpMsgEnd, strlen(helpMsgEnd));

	/* RNFR <SP> <pathname> <CRLF> */
      } else if (strcmp(arg, "RNFR") == 0) {

	strLength = snprintf(cmdLine, (CMD_STRING_LENGTH + 1), "To use: %s <SP> <pathname> <CRLF>\n", arg);
	strncpy(cmdInfo, "\tUsed when renaming a file; rename from\n", (CMD_STRING_LENGTH + 1));

	send_all(si->c_sfd, (uint8_t *)helpMsgStart, strlen(helpMsgStart));
	send_all(si->c_sfd, (uint8_t *)cmdLine, strLength);
	send_all(si->c_sfd, (uint8_t *)cmdInfo, strlen(cmdInfo));
	send_all(si->c_sfd, (uint8_t *)helpMsgEnd, strlen(helpMsgEnd));

	/* RNTO <SP> <pathname> <CRLF> */
      } else if (strcmp(arg, "RNTO") == 0) {

	strLength = snprintf(cmdLine, (CMD_STRING_LENGTH + 1), "To use: %s <SP> <pathname> <CRLF>\n", arg);
	strncpy(cmdInfo, "\tUsed when renaming a file; rename to\n", (CMD_STRING_LENGTH + 1));

	send_all(si->c_sfd, (uint8_t *)helpMsgStart, strlen(helpMsgStart));
	send_all(si->c_sfd, (uint8_t *)cmdLine, strLength);
	send_all(si->c_sfd, (uint8_t *)cmdInfo, strlen(cmdInfo));
	send_all(si->c_sfd, (uint8_t *)helpMsgEnd, strlen(helpMsgEnd));

	/* DELE <SP> <pathname> <CRLF> */
      } else if (strcmp(arg, "DELE") == 0) {

	strLength = snprintf(cmdLine, (CMD_STRING_LENGTH + 1), "To use: %s <SP> <pathname> <CRLF>\n", arg);
	strncpy(cmdInfo, "\tDelete a remote file\n", (CMD_STRING_LENGTH + 1));

	send_all(si->c_sfd, (uint8_t *)helpMsgStart, strlen(helpMsgStart));
	send_all(si->c_sfd, (uint8_t *)cmdLine, strLength);
	send_all(si->c_sfd, (uint8_t *)cmdInfo, strlen(cmdInfo));
	send_all(si->c_sfd, (uint8_t *)helpMsgEnd, strlen(helpMsgEnd));

	/* SITE <SP> <string> <CRLF> */
      } else if (strcmp(arg, "SITE") == 0) {

	strLength = snprintf(cmdLine, (CMD_STRING_LENGTH + 1), "To use: %s <SP> <string> <CRLF>\n", arg);
	strncpy(cmdInfo, "\tSend site specific command to the remote server\n", (CMD_STRING_LENGTH + 1));

	send_all(si->c_sfd, (uint8_t *)helpMsgStart, strlen(helpMsgStart));
	send_all(si->c_sfd, (uint8_t *)cmdLine, strLength);
	send_all(si->c_sfd, (uint8_t *)cmdInfo, strlen(cmdInfo));
	send_all(si->c_sfd, (uint8_t *)helpMsgEnd, strlen(helpMsgEnd));

	/* STAT [<SP> <pathname>] <CRLF> */
      } else if (strcmp(arg, "STAT") == 0) {

	strLength = snprintf(cmdLine, (CMD_STRING_LENGTH + 1), "To use: %s [<SP> <pathname>] <CRLF>\n", arg);
	strncpy(cmdInfo, "\tReturn current server status\n", (CMD_STRING_LENGTH + 1));

	send_all(si->c_sfd, (uint8_t *)helpMsgStart, strlen(helpMsgStart));
	send_all(si->c_sfd, (uint8_t *)cmdLine, strLength);
	send_all(si->c_sfd, (uint8_t *)cmdInfo, strlen(cmdInfo));
	send_all(si->c_sfd, (uint8_t *)helpMsgEnd, strlen(helpMsgEnd));

      } else {

	send_all(si->c_sfd, (uint8_t *)argUnrecognized, strlen(argUnrecognized));

      } //END statement 'if-else'

    } else if (strlen(arg) == MIN_CMD_SIZE) {

      /* CWD <SP> <pathname> <CRLF> */
      if (strcmp(arg, "CWD") == 0) {

	strLength = snprintf(cmdLine, (CMD_STRING_LENGTH + 1), "To use: %s <SP> <pathname> <CRLF>\n", arg);
	strncpy(cmdInfo, "\tChange the current working directory on the remote machine\n", (CMD_STRING_LENGTH + 1));

	send_all(si->c_sfd, (uint8_t *)helpMsgStart, strlen(helpMsgStart));
	send_all(si->c_sfd, (uint8_t *)cmdLine, strLength);
	send_all(si->c_sfd, (uint8_t *)cmdInfo, strlen(cmdInfo));
	send_all(si->c_sfd, (uint8_t *)helpMsgEnd, strlen(helpMsgEnd));

	/* RMD <SP> <pathname> <CRLF> */
      } else if (strcmp(arg, "RMD") == 0) {

	strLength = snprintf(cmdLine, (CMD_STRING_LENGTH + 1), "To use: %s <SP> <pathname> <CRLF>\n", arg);
	strncpy(cmdInfo, "\tRemove a directory on the remote machine\n", (CMD_STRING_LENGTH + 1));

	send_all(si->c_sfd, (uint8_t *)helpMsgStart, strlen(helpMsgStart));
	send_all(si->c_sfd, (uint8_t *)cmdLine, strLength);
	send_all(si->c_sfd, (uint8_t *)cmdInfo, strlen(cmdInfo));
	send_all(si->c_sfd, (uint8_t *)helpMsgEnd, strlen(helpMsgEnd));

	/* MKD <SP> <pathname> <CRLF> */
      } else if (strcmp(arg, "MKD") == 0) {

	strLength = snprintf(cmdLine, (CMD_STRING_LENGTH + 1), "To use: %s <SP> <pathname> <CRLF>\n", arg);
	strncpy(cmdInfo, "\tCreate a directory on the remote machine\n", (CMD_STRING_LENGTH + 1));

	send_all(si->c_sfd, (uint8_t *)helpMsgStart, strlen(helpMsgStart));
	send_all(si->c_sfd, (uint8_t *)cmdLine, strLength);
	send_all(si->c_sfd, (uint8_t *)cmdInfo, strlen(cmdInfo));
	send_all(si->c_sfd, (uint8_t *)helpMsgEnd, strlen(helpMsgEnd));

	/* PWD <CRLF> */
      } else if (strcmp(arg, "PWD") == 0) {

	strLength = snprintf(cmdLine, (CMD_STRING_LENGTH + 1), "To use: %s <CRLF>\n", arg);
	strncpy(cmdInfo, "\tPrint the current working directory on the remote machine\n", (CMD_STRING_LENGTH + 1));

	send_all(si->c_sfd, (uint8_t *)helpMsgStart, strlen(helpMsgStart));
	send_all(si->c_sfd, (uint8_t *)cmdLine, strLength);
	send_all(si->c_sfd, (uint8_t *)cmdInfo, strlen(cmdInfo));
	send_all(si->c_sfd, (uint8_t *)helpMsgEnd, strlen(helpMsgEnd));

      } else {

	send_all(si->c_sfd, (uint8_t *)argUnrecognized, strlen(argUnrecognized));

      } //END statement 'if-else'

    } else {

      send_all(si->c_sfd, (uint8_t *)argUnrecognized, strlen(argUnrecognized));

    } //END statement 'if-else'

    free(cmdLine);
    free(cmdInfo);

    return 0;

  } //END statement 'if-else'

} //END function 'command_help'
