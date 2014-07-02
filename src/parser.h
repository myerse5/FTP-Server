/******************************************************************************
 * FTP-Server
 * Authors: Evan Myers, Justin Slind, Alex Tai, James Yoo
 * Date: November 2013
 *
 * Description:
 *   The following functions perform parsing operations on an input string that
 *   contains an FTP command along with all of its relevant arguments.
 *****************************************************************************/
#ifndef __PARSER_H__
#define __PARSER_H__


/******************************************************************************
 * Separate the first token (the command) from all other tokens (the arguments).
 * The string passed to this function will be set to the command token, and any
 * arguments appearing after the command will be returned by the function as a
 * string. The caller function must free the returned string as it was created
 * using a call to malloc().
 *
 * Return values:
 *   NULL  - There are no arguments.
 *   char* - The argument string.
 *****************************************************************************/
char *separate_cmd_from_args (char **cmdLineStr, int numArgs);


/******************************************************************************
 * Count the number of arguments that follow a command from a command received
 * on the control connection from a client. Any leading or trailing whitespace
 * must be removed before calling this function to receive an accurate count.
 *
 * Returns: The number of arguments found after a command.
 *****************************************************************************/
int get_arg_count (const char *cmdString);


/******************************************************************************
 * Convert all lower case characters in a string to uppercase.
 *****************************************************************************/
void convert_to_upper (char *string);

#endif //__PARSER_H__
