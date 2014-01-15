/******************************************************************************
 * Students: Evan Myers, Justin Slind, Alex Tai, James Yoo
 * Course: CMPT-361
 * Assignment #3 - ftp server
 * File: parser.h
 * Date: November 2013
 *
 * Description:
 *   The following functions perform parsing operations on an input string that
 *   contains an FTP command along with all of its relevant arguments.
 *****************************************************************************/
#ifndef __PARSER_H__
#define __PARSER_H__


/******************************************************************************
 * This function probably counts the number of arguments. No documentation.
 *
 * Original author: James Yoo
 *****************************************************************************/
int command_arg_count(const char *cmdString);


/******************************************************************************
 * This function probably extracts an argument. No documentation.
 *
 * Original author: James Yoo
 *****************************************************************************/
char *command_extract_arg(const char *cmdString);


/******************************************************************************
 * This function probably extracts a command. No documentation.
 *
 * Original author: James Yoo
 *****************************************************************************/
char *command_extract_cmd(const char *cmdString);


/******************************************************************************
 * An achronym for the function name. No documentation. very bad combination.
 *
 * Original author: James Yoo
 *****************************************************************************/
char *strdup(const char *string);


/******************************************************************************
 * Probably converts to uppercase. No documentation.
 *
 * Original author: James Yoo
 *****************************************************************************/
void convert_to_upper(char *string);


/******************************************************************************
 * Probably trims whitespace. No documentation.
 *
 * Original author: James Yoo
 *
 * Acknowledgement:
 * http://www.stackoverflow.com/questions/122616/how-do-i-trim-leading-trailing-whitespace-in-a-standard-way/
 *****************************************************************************/
void trim_whitespace(char *string);


#endif //__PARSER_H__
