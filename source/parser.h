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
 * No documentation.
 *
 * Original author: James Yoo
 *****************************************************************************/
int command_arg_count (const char *cmdString);


/******************************************************************************
 * No documentation.
 *
 * Original author: James Yoo
 *****************************************************************************/
char *command_extract_arg (const char *cmdString);


/******************************************************************************
 * No documentation.
 *
 * Original author: James Yoo
 *****************************************************************************/
char *command_extract_cmd (const char *cmdString);


/******************************************************************************
 * No documentation. String duplicate probably.
 *
 * Original author: James Yoo
 *****************************************************************************/
char *strdup (const char *string);


/******************************************************************************
 * No documentation.
 *
 * Original author: James Yoo
 *****************************************************************************/
void convert_to_upper (char *string);


/******************************************************************************
 * No documentation.
 *
 * Original author: James Yoo
 *****************************************************************************/
void trim_whitespace (char *string);


#endif //__PARSER_H__
