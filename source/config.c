/******************************************************************************
 * Students: Evan Myers, Justin Slind, Alex Tai, James Yoo
 * Course: CMPT-361
 * Assignment #3 - ftp server
 * File: config.c
 * Date: November 2013
 *
 * Description:
 *   Functions that work with the server configuration file, ftp.conf, are
 *   found in this file.
 *
 * Acknowledgements:
 *   Evan - The decision to include the configuration file was made after a
 *          conversation with Dr. Nicholas Boers.
 *****************************************************************************/
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "config.h"


//The maximum length of a line found in the config file.
#define MAX_CONFIG_LINE 80
//This character begins a comment line in the configuration file.
#define CONFIG_COMMENT '#'


//Local function prototype.
static char *search_config (const char *target, const char *pathname);


/******************************************************************************
 * get_config_value - see config.h
 *****************************************************************************/
char *get_config_value (const char *config_setting, const char *filen)
{
  char *config_path = NULL;
  char *result_value;

  //Get the filename path to the configuration file.
  if ((config_path = get_config_path (filen)) == NULL) {
    return NULL;
  }

  //Retrieve the value of the setting from the configuration file.
  if ((result_value = search_config ((const char *)config_setting,
				     (const char *)config_path)) == NULL) {
    free (config_path);
    return NULL;
  }

  //Free all memory no longer required, return the found value.
  free (config_path);
  return result_value;
}


/******************************************************************************
 * Get the absolute pathname for the configuration file of the server. The
 * first argument passed to this function will be set to this value on function
 * return.
 *
 * The caller function must be sure to free the memory returned by this
 * function.
 *
 * Arguments:
 *   filen - return the full pathname for this configuration file.
 *
 * Returns:
 *   A pointer to the absolute pathname string. If NULL is returned, there was
 *   an error, and the pathname could not be determined.
 *
 * Original author: Evan Myers
 *
 * Acknowledgements:
 *   This function is quite similar to the opening portion of Evan's function
 *   'client_file_setup' in the file 'fileop.c' submitted in his second
 *   assignment.
 *****************************************************************************/
char *get_config_path (const char *filen)
{
  int filen_sz, path_sz, abs_path_sz;    //The required string lengths.
  char *path;

  //Get the pathname of the current working directory.
  if ((path = getcwd (NULL, 0)) == NULL) {
    fprintf (stderr, "%s: getcwd: %s\n", __FUNCTION__, strerror (errno));
    return NULL;
  }

  /* Calculate the size of string required for the absolute pathname of the
   * file being created in this block. Using these values we will realloc()
   * the path, and append the configuration file name. */
  filen_sz = strlen (filen);
  path_sz = strlen (path);
  /* We will replace the null character for the pathname with a directory
   * seperator '/', and must null-terminate the string. Therefore, we will
   * add one to each value returned from strlen() as strlen() does not include
   * the null terminator. */
  abs_path_sz = (filen_sz + 1) + (path_sz + 1);

  //Adjust the size of the pathname to include the filename being created.
  if ((path = realloc (path, abs_path_sz)) == NULL) {
    fprintf (stderr, "%s: realloc: could not allocate required space\n",
	     __FUNCTION__);
    return NULL;
  }

  /* Replace the null character appearing after the path with a directory
   * seperator '/'. Then append the filename. */
  path[path_sz] = '/';
  //Append the filename to the current working directory path.
  strncpy ((path + path_sz + 1), filen, (filen_sz + 1));
  
  return path;
}


/******************************************************************************
 * Search the contents of the configuration file for the setting passed in the
 * first argument to the function. The string that is returned from this
 * function will be the value that was set in the configuration file for the
 * target setting.
 *
 * The caller function must be sure to free the memory returned by this
 * function.
 *
 * Arguments:
 *   target   - The setting to be searched for in the configuration file.
 *   pathname - The absolute path to the configuration file.
 *
 * Returns:
 *    NULL - Error, the setting was not present in the configuration file, or
 *           there has been an error during a system call.
 *  string - The value of the setting found in the configuration file.
 *
 * Original author: Evan Myers
 *****************************************************************************/
static char *search_config (const char *target, const char *pathname)
{
  FILE *fin;                      //Filepointer for the config file.
 
  char line[MAX_CONFIG_LINE + 1]; //Collect a line from the config file.
  char *line_ptr;

  char *value;                    //The value of the target setting.
  int val_length; //The length of the value string for the target setting.

  //Open the filestream.
  if ((fin = fopen (pathname, "r")) == NULL) {
    fprintf (stderr, "%s: fopen: %s\n", __FUNCTION__, strerror (errno));
    return NULL;
  }

  //Search each line for the target setting. 
  while (fgets (line, MAX_CONFIG_LINE, fin) != NULL) {
    /* The configuration file allows comments. View the header comment of the
     * the configuration file for more details. Skip any comment lines. */
    if (line[0] == CONFIG_COMMENT)
      continue;

    //Move to the next line if this line is not the setting.
    if (strstr (line, target) == NULL)
      continue;   //not present, search next line.
    else
      break;
  }

  /* The filestream is no longer required after this point, close it. */
  if (fclose (fin) == -1) {
    fprintf (stderr, "%s: fclose: %s\n", __FUNCTION__, strerror (errno));
    return NULL;
  }

  //Determine if the target setting was found or the entire file was read.
  if (strstr (line, target) == NULL) {
    fprintf (stderr, "%s: '%s' setting was not found in file './%s'\n",
	     __FUNCTION__, target, pathname);
    return NULL;
  }

  /* The target setting was found in the config file, determine the amount of
   * memory to allocate so that the value of the target setting can be returned
   * to the caller function as a string. */
  if ((line_ptr = strchr (line, ' ')) == NULL) {
    fprintf (stderr, "%s: stchr: improper config file format\n", __FUNCTION__);
    return NULL;
  }

  //Do not include the space character seperating the target setting and value.
  line_ptr++;
  //Find the length of the value for the target setting.
  val_length = strlen (line_ptr);

  /* Return error if the value contains no characters. This condition will be
   * true if the last line of the configuration file includes a space but no
   * value for the setting and:
   *    -The last line is the setting being searched for.
   *    -The setting was not found in the file. */
  if (val_length == 0) {
    fprintf (stderr, "%s: stchr: improper config file format\n", __FUNCTION__);
    return NULL;
  } 

  //Return error if the only character in the value is a newline character.
  if ((val_length == 1) && (line_ptr[0] == '\n')) {
    fprintf (stderr, "%s: stchr: improper config file format\n", __FUNCTION__);
    return NULL;
  }

  /* Remove the newline character at the end of the value if present. The
   * newline character may not be present if the setting was the last line of
   * the file. */
  if (line_ptr[val_length - 1] == '\n') {
    line_ptr[val_length - 1] = '\0';
    val_length--;
  }

  //Allocate memory for the value string.
  if ((value = malloc ((val_length + 1) * sizeof (*value))) == NULL) {
    fprintf (stderr, "%s: malloc: could not allocate the required space\n",
	     __FUNCTION__);
    return NULL;
  }
  //Copy the value from the target setting line to the target setting value str.
  strncpy (value, line_ptr, val_length + 1);

  return value;
}
