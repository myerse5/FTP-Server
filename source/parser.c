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


/******************************************************************************
 * command_arg_count - see "parser.h"
 *****************************************************************************/
int command_arg_count (const char *cmdString)
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
 * command_extract_arg - see "parser.h"
 *****************************************************************************/
char *command_extract_arg (const char *cmdString)
{
  int argCount;
  int tempLen;
  int argLen;

  char *argString; 
  char *command;
  char *tempString;

  argCount = command_arg_count (cmdString);

  if (argCount < MIN_NUM_ARGS) {
    return NULL;
  }

  tempString = strdup (cmdString);
  tempLen = strlen (tempString) + 1; //+1 for a null terminator.

  if ((argString = malloc (tempLen * sizeof(char))) == NULL) {
    fprintf (stderr, 
	     "%s: malloc: could not allocate required space\n", __FUNCTION__);
    free (tempString);
    return NULL;
  }

  command = command_extract_cmd (cmdString);
  memcpy (argString, (tempString + strlen (command)), 
	  ((strlen (tempString) - strlen (command)) * sizeof(char)));

  trim_whitespace (argString);
  argLen = strlen (argString) + 1; //+1 for a null terminator.

  if ((argString = realloc (argString, argLen * sizeof(char))) == NULL) {
    fprintf (stderr, 
	     "%s: realloc: could not allocate required space\n", __FUNCTION__);
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
  char *command;
  char *tempString;
  char *token;
  int trimLen;
  int tokLen;


  tempString = strdup (cmdString);
  trimLen = strlen (tempString) + 1; //+1 for a null terminator.

  if ((command = malloc (trimLen * sizeof(char))) == NULL) {
    fprintf (stderr, 
	     "%s: malloc: could not allocate required space\n", __FUNCTION__);
    free (tempString);
    return NULL;
  }

  token = strtok (tempString, " ");
  tokLen = strlen (token) + 1; //+1 for a null terminator.

  if ((command = realloc (command, tokLen * sizeof(char))) == NULL) {
    fprintf (stderr, 
	     "%s: realloc: could not allocate required space\n", __FUNCTION__);
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
  int strLen;

  strLen = strlen (string) + 1; //+1 for null terminator.

  if ((duplicate = malloc (strLen * sizeof(char))) == NULL) {
    fprintf (stderr,
	     "%s: malloc: could not allocate required space\n", __FUNCTION__);
    return NULL;
  }

  strcpy (duplicate, string);
  trim_whitespace (duplicate);

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
