/******************************************************************************
 * Authors: Evan Myers, Justin Slind, Alex Tai, James Yoo
 * FTP-Server
 * Date: November 2013
 *
 * Description:
 *   The following functions perform parsing operations on an input string that
 *   contains an FTP command along with all of its relevant arguments.
 *****************************************************************************/
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "parser.h"


/* To extract an argument from the command string there must be at least two
 * tokens (cmd + arg + ...). */
#define MIN_NUM_ARGS 2 


//Local function prototypes.
static void trim_whitespace (char *string);


/******************************************************************************
 * separate_cmd - see "parser.h"
 *****************************************************************************/
char *separate_cmd_from_args (char **cmdLineStr, int numArgs)
{
  char *args;

  trim_whitespace (*cmdLineStr);

  if (numArgs == 1) {
    convert_to_upper (*cmdLineStr);
    return NULL;
  }
  args = *cmdLineStr;

  //Find the next whitespace character.
  while (!isspace (*(args++)));

  /* Replace the whitespace character with a NULL terminator to separate the
   * command and argument strings. */
  *(args - 1) = '\0';
  convert_to_upper (*cmdLineStr);

  return args;
}


/******************************************************************************
 * command_arg_count - see "parser.h"
 *****************************************************************************/
int get_arg_count (const char *cmdString)
{
  int argCount = 0;

  for (int i = 0; i < strlen (cmdString); i++) {
    if (!isspace (cmdString[i])) {
      while (!isspace (cmdString[i]) && (i < strlen (cmdString))) {
	i++;
      }
      argCount++;
    }
  }
  return argCount;
}


/******************************************************************************
 * convert_to_upper - see "parser.h"
 *****************************************************************************/
void convert_to_upper (char *string)
{
  int i;

  for (i = 0; i < strlen (string); i++) {
    string[i] = toupper (string[i]);
  }
}


/******************************************************************************
 * Remove any leading/trailing whitespace from a string.
 *
 * Original author: James Yoo
 *****************************************************************************/
static void trim_whitespace (char *string)
{
  int length;
  char *head;
  char *tail;

  length = strlen (string);
  head = string - 1;
  tail = string + length;

  //Point the head pointer to the first non-whitespace character.
  while (isspace (*(++head)));
  //Point the tail pointer to the last non-whitespace character.
  while (isspace (*(--tail)) && (tail != head));

  //Trim trailing whitespace.
  if ((string + length - 1) != tail) {
    *(tail + 1) = '\0';
  } else if ((head != string) && (tail == head)) {
    *string = '\0';
  }

  tail = string;

  //Trim leading whitespace.
  if (head != string) {
    while (*head) *tail++ = *head++;
    *tail = '\0';
  }
}
