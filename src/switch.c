/******************************************************************************
 * FTP-Server
 * Authors: Evan Myers, Justin Slind, Alex Tai, James Yoo
 * Date: November 2013
 *
 * Description:
 *   The command thread is created by calling 'pthread_create()' and using a
 *   function pointer to command_switch. The input received on the control
 *   connection is passed with the 'pthread_create()' call. The function
 *   command_switch() calls the appropriate command that was requested by the
 *   input data captured from the client.
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
#include "log.h"
#include "net.h"
#include "reply.h"
#include "session.h"
#include "user.h"


// Ensure a command has been sent (not an empty line).
#define MIN_NUM_ARGS 1    // Minimum allowed number of arguments.


/******************************************************************************
 * command_switch - see "switch.h"
 *****************************************************************************/
void *command_switch (void *param) {
  session_info_t *si;  // The data for this session (sockets, user, etc.)
  char *cmdLine;       // The full command received from the client on the csfd.
  char *cmd;           // The requested command.
  char *arg;           // The argument to use with the command.
  int numArgs;         // The number of tokens in the variable 'cmdLine'.
  int csfd;            // Control socket file descriptor.

  si = (session_info_t *)param;
  cmdLine = si->cmdString;
  numArgs = get_arg_count (cmdLine);
  csfd = si->csfd;

  if (numArgs < MIN_NUM_ARGS) {
    log_received_cmd (si->user, NULL, NULL, 0);
    send_mesg_500 (csfd);
    si->cmdComplete = true;
    return NULL;
  }

  arg = separate_cmd_from_args (&cmdLine, numArgs);
  cmd = cmdLine;
  log_received_cmd (si->user, cmd, arg, numArgs);

  // APPE <SP> <pathname> <CRLF>
  if (strcmp (cmd, "APPE") == 0) {
    cmd_appe (si, arg);

    // CDUP <CRLF>
  } else if (strcmp (cmd, "CDUP") == 0) {
    cmd_cdup (si, arg);

    // CWD <SP> <pathname> <CRLF>   
  } else if (strcmp (cmd, "CWD") == 0) {
    cmd_cwd (si, arg);

    // HELP [<SP> <string>] <CRLF>
  } else if (strcmp (cmd, "HELP") == 0) {
    if (arg != NULL)
      convert_to_upper (arg);
    cmd_help (si, arg);

    // LIST [<SP> <pathname>] <CRLF>
  } else if (strcmp (cmd, "LIST") == 0) {
    cmd_list_nlst (si, arg, true);

    // MKD <SP> <pathname> <CRLF>
  } else if (strcmp (cmd, "MKD") == 0) {
    cmd_mkd (si, arg);

    // MODE <SP> <mode-code> <CRLF>
  } else if (strcmp (cmd, "MODE") == 0) {
    cmd_mode (si, arg);

    // NLST [<SP> <pathname>] <CRLF>
  } else if (strcmp (cmd, "NLST") == 0) {
    cmd_list_nlst (si, arg, false);

    // PASS <SP> <password> <CRLF>
  } else if (strcmp (cmd, "PASS") == 0) {
    cmd_pass (si, arg);

    // PASV <CRLF>
  } else if (strcmp (cmd, "PASV") == 0) {
    cmd_pasv (si);

    // PORT <SP> <host-port> <CRLF>
  } else if (strcmp (cmd, "PORT") == 0) {
    cmd_port (si, arg);

    // PWD <CRLF>
  } else if (strcmp (cmd, "PWD") == 0) {
    cmd_pwd (si);

    // QUIT <CRLF>
  } else if (strcmp (cmd, "QUIT") == 0) {
    cmd_quit (si);

    // RETR <SP> <pathname> <CRLF>
  } else if (strcmp (cmd, "RETR") == 0) {
    cmd_retr (si, arg);

    // STOR <SP> <pathname> <CRLF>
  } else if (strcmp (cmd, "STOR") == 0) {
    cmd_stor (si, arg);

    // STOU <CRLF>
  } else if (strcmp (cmd, "STOU") == 0) {
    cmd_stou (si, arg);

    // STRU <SP> <structure-code> <CRLF>
  } else if (strcmp (cmd, "STRU") == 0) {
    cmd_stru (si, arg, numArgs);

    // SYST <CRLF>
  } else if (strcmp (cmd, "SYST") == 0) {
    cmd_syst (si);

    // TYPE <SP> <type-code> <CRLF>
  } else if (strcmp (cmd, "TYPE") == 0) {
    cmd_type (si, arg);

    // USER <SP> <username> <CRLF>
  } else if (strcmp (cmd, "USER") == 0) {
    cmd_user (si, arg);

    // ABOR <CRLF>
  } else if (strcmp (cmd, "ABOR") == 0) {
    send_mesg_504 (csfd);

    // ACCT <SP> <account-information> <CRLF>
  } else if (strcmp (cmd, "ACCT") == 0) {
    send_mesg_504 (csfd);

    // ALLO <SP> <decimal-integer> [<SP> R <SP> <decimal-integer>] <CRLF>
  } else if (strcmp (cmd, "ALLO") == 0) {
    send_mesg_504 (csfd);

    // DELE <SP> <pathname> <CRLF>
  } else if (strcmp (cmd, "DELE") == 0) {
    send_mesg_504 (csfd);

    // NOOP <CRLF>
  } else if (strcmp (cmd, "NOOP") == 0) {
    send_mesg_504 (csfd);

    // REIN <CRLF>
  } else if (strcmp (cmd, "REIN") == 0) {
    send_mesg_504 (csfd);

    // REST <SP> <marker> <CRLF>
  } else if (strcmp (cmd, "REST") == 0) {
    send_mesg_504 (csfd);

    // RMD <SP> <pathname> <CRLF>
  } else if (strcmp (cmd, "RMD") == 0) {
    send_mesg_504 (csfd);

    // RNFR <SP> <pathname> <CRLF>
  } else if (strcmp (cmd, "RNFR") == 0) {
    send_mesg_504 (csfd);

    // RNTO <SP> <pathname> <CRLF>
  } else if (strcmp (cmd, "RNTO") == 0) {
    send_mesg_504 (csfd);

    // SITE <SP> <string> <CRLF>
  } else if (strcmp (cmd, "SITE") == 0) {
    send_mesg_504 (csfd);

    // SMNT <SP> <pathname> <CRLF>
  } else if (strcmp (cmd, "SMNT") == 0) {
    send_mesg_504 (csfd);

    // STAT [<SP> <pathname>] <CRLF>
  } else if (strcmp (cmd, "STAT") == 0) {
    send_mesg_504 (csfd);

  } else {
    send_mesg_500 (csfd);
  }

  si->cmdComplete = true;
  return NULL;
}
