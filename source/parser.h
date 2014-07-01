/******************************************************************************
 * Authors: Evan Myers, Justin Slind, Alex Tai, James Yoo
 * FTP-Server
 * Date: November 2013
 *
 * Description:
 *   The following functions perform parsing operations on an input string that
 *   contains an FTP command along with all of its relevant arguments.
 *****************************************************************************/
#ifndef __PARSER_H__
#define __PARSER_H__


/******************************************************************************
 * Count the number of arguments that follow a command from a command received
 * on the control connection from a client. Any leading or trailing whitespace
 * must be removed before calling this function to receive an accurate count.
 *
 * Returns: The number of arguments found after a command.
 *
 * Original author: James Yoo
 *****************************************************************************/
int get_arg_count (const char *cmdString);


/******************************************************************************
 * TODO: malloc/realloc called 6 to get the argument string. Fix this.
 *
 * Extract a string containing all arguments found after the first token from
 * the input received from a client over the control connection. The returned
 * string will have all leading/trailing whitespace removed.
 *
 * The returned string must be freed by the caller.
 *
 * Original author: James Yoo
 *****************************************************************************/
char *extract_arg_string (const char *cmdString);


/******************************************************************************
 * TODO: malloc/realloc is called 3 times to retrieve the first token. Fix this.
 *
 * Extract the first token (the command) from the input received from a client
 * over the control connection. The returned string will have all
 * leading/trailing whitespace removed.
 *
 * The returned string must be freed by the caller.
 *
 * Original author: James Yoo
 *****************************************************************************/
char *extract_cmd_string (const char *cmdString);


#endif //__PARSER_H__
