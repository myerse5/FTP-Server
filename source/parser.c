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
#include <stdlib.h>
#include <string.h>
#include "parser.h"


//The minimum allowed number of arguments for command_extract_arg().
#define MIN_NUM_ARGS 2 

/******************************************************************************
 * command_arg_count - see "parser.h"
 *****************************************************************************/
int command_arg_count (const char *cmdString)
{
  int argCount;

  argCount = 0;

  for (int i = 0; i < strlen(cmdString); i++) {
    if (!isspace(cmdString[i])) {
      while (!isspace(cmdString[i]) && (i < strlen(cmdString))) {
	i++;
      }
      argCount++;
    }
  }
  return argCount;
}


/******************************************************************************
 * command_extract_arg - see "parser.h"
 *****************************************************************************/
char *command_extract_arg (const char *cmdString)
{
  int argCount;

  char *argString, 
       *command,
       *tempString;

  argCount = command_arg_count(cmdString);

  if (argCount < MIN_NUM_ARGS) {
    return NULL;
  }

  tempString = strdup(cmdString);

  if ((argString = (char *)calloc((strlen(tempString) + 1), sizeof(char))) == NULL) {
    free(tempString);
    return NULL;
  }

  command = command_extract_cmd (cmdString);
  memcpy (argString, (tempString + strlen (command)), ((strlen (tempString) - strlen (command)) * sizeof(char)));
  trim_whitespace (argString);

  if ((argString = (char *)realloc (argString, ((strlen (argString) + 1) * sizeof(char)))) == NULL) {
    free (command);
    free (tempString);
    return NULL;
  }

  free (command);
  free (tempString);

  return argString;
}


/******************************************************************************
 * command_extract_cmd - see "parser.h"
 *****************************************************************************/
char *command_extract_cmd (const char *cmdString)
{
  char *command,
       *tempString,
       *token;

  tempString = strdup(cmdString);

  if ((command = (char *)calloc ((strlen (tempString) + 1), sizeof(char))) == NULL) {
    free (tempString);
    return NULL;
  }

  token = strtok(tempString, " ");

  if ((command = (char *)realloc (command, ((strlen(token) + 1) * sizeof(char)))) == NULL) {
    free (tempString);
    return NULL;
  }

  strcpy (command, token);
  convert_to_upper (command);

  free (tempString);

  return command;
}


/******************************************************************************
 * strdup - see "parser.h"
 *****************************************************************************/
char *strdup (const char *string)
{
  char *duplicate;

  if ((duplicate = (char *)calloc ((strlen (string) + 1), sizeof(char))) == NULL) {
    return NULL;
  }

  strcpy(duplicate, string);
  trim_whitespace(duplicate);

  return duplicate;
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
 * trim_whitespace - see "parser.h"
 *****************************************************************************/
void trim_whitespace (char *string)
{
  int length;

  char *head,
       *tail;

  length = strlen (string);
  head = string - 1;
  tail = string + length;

  while (isspace (*(++head)));
  while (isspace (*(--tail)) && (tail != head));

  if ((string + length - 1) != tail) {
    *(tail + 1) = '\0';
  } else if ((head != string) && (tail == head)) {
    *string = '\0';
  }

  tail = string;

  if (head != string) {
    while (*head) *tail++ = *head++;
    *tail = '\0';
  }
}
