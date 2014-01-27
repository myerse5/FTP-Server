/******************************************************************************
 * Students: Evan Myers, Justin Slind, Alex Tai, James Yoo
 * Course: CMPT-361
 * Assignment #3 - ftp server
 * File: parser.c
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
int command_arg_count(const char *cmdString)
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
char *command_extract_arg(const char *cmdString)
{ //BEGIN function 'command_extract_arg'

  int argCount;

  char *argString, 
       *command,
       *tempString;

  argCount = command_arg_count(cmdString);

  if (argCount < MIN_NUM_ARGS) {
    return NULL;
  } //END statement 'if'

  tempString = strdup(cmdString);

  if ((argString = (char *)calloc((strlen(tempString) + 1), sizeof(char))) == NULL) {
    free(tempString);
    return NULL;
  } //END statement 'if'

  command = command_extract_cmd(cmdString);
  memcpy(argString, (tempString + strlen(command)), ((strlen(tempString) - strlen(command)) * sizeof(char)));
  trim_whitespace(argString);

  if ((argString = (char *)realloc(argString, ((strlen(argString) + 1) * sizeof(char)))) == NULL) {
    free (command);
    free (tempString);
    return NULL;
  } //END statement 'if'

  free(command);
  free(tempString);

  return argString;

} //END function 'command_extract_arg'


/******************************************************************************
 * command_extract_cmd - see "parser.h"
 *****************************************************************************/
char *command_extract_cmd(const char *cmdString)
{ //BEGIN function 'command_extract'

  char *command,
       *tempString,
       *token;

  tempString = strdup(cmdString);

  if ((command = (char *)calloc((strlen(tempString) + 1), sizeof(char))) == NULL) {
    free(tempString);
    return NULL;
  } //END statement 'if'

  token = strtok(tempString, " ");

  if ((command = (char *)realloc(command, ((strlen(token) + 1) * sizeof(char)))) == NULL) {
    free(tempString);
    return NULL;
  } //END statement 'if'

  strcpy(command, token);
  convert_to_upper(command);

  free(tempString);

  return command;

} //END function 'command_extract'


/******************************************************************************
 * strdup - see "parser.h"
 *****************************************************************************/
char *strdup(const char *string)
{ //BEGIN function 'strdup'

  char *duplicate;

  if ((duplicate = (char *)calloc((strlen(string) + 1), sizeof(char))) == NULL) {
    return NULL;
  } //END statement 'if'

  strcpy(duplicate, string);
  trim_whitespace(duplicate);

  return duplicate;

} //END function 'strdup'


/******************************************************************************
 * convert_to_upper - see "parser.h"
 *****************************************************************************/
void convert_to_upper(char *string)
{ //BEGIN function 'conver_to_upper'

  for (int i = 0; i < strlen(string); i++) {
    string[i] = toupper(string[i]);
  } //END loop 'for'

} //END function 'convert_to_upper'


/******************************************************************************
 * trim_whitespace - see "parser.h"
 *****************************************************************************/
void trim_whitespace(char *string)
{ //BEGIN function 'trim_whitespace'

  int length;

  char *head,
       *tail;

  length = strlen(string);
  head = string - 1;
  tail = string + length;

  while (isspace(*(++head)));
  while (isspace(*(--tail)) && (tail != head));

  if ((string + length - 1) != tail) {
    *(tail + 1) = '\0';
  } else if ((head != string) && (tail == head)) {
    *string = '\0';
  } //END statement 'if-else'

  tail = string;

  if (head != string) {
    while (*head) *tail++ = *head++;
    *tail = '\0';
  } //END statement 'if'

} //END function 'trim_whitespace'
