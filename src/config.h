/******************************************************************************
 * Authors: Evan Myers, Justin Slind, Alex Tai, James Yoo
 * FTP-Server
 * Date: November 2013
 *
 * Description:
 *   Functions that operate on the server configuration file, ftp.conf, are
 *   found in this file.
 *****************************************************************************/
#ifndef __CONFIG_H__
#define __CONFIG_H__


//The name of the server configuration file.
#define FTP_CONFIG_FILE "../conf/ftp.conf"
//The name of the user configuration file.
#define USER_CONFIG_FILE "../conf/user.conf"


/******************************************************************************
 * This function will open the configuration file, search for the setting
 * passed in the first argument, and return the value that has been set for
 * this target setting. 
 *
 * The function caller must be sure to free the string returned by this
 * function (the returned string was made using malloc).
 *
 * Arguments:
 *   configSetting - Retrieve the value of this setting from the config file.
 *
 *   filename - return the full pathname for the configuration file
 *
 * Returns:
 *   Return a pointer to the string of all characters found after the target
 *   setting in the config file.
 *
 *   This string begins after the space character found after the target
 *   setting. The string ends at the newline character. This string is
 *   null-terminated.
 *
 *   If NULL is returned, there was an error. View the standard error log for
 *   the cause of this error.
 *
 * Original author: Evan Myers
 *****************************************************************************/
char *get_config_value (const char *configSetting, const char *filename);


/******************************************************************************
 * Get the absolute pathname for the configuration file of the server. The
 * first argument passed to this function will be set to this value on function
 * return.
 *
 * The caller function must be sure to free the memory returned by this
 * function.
 *
 * Arguments:
 *   filename - return the full pathname for the configuration file.
 *
 * Returns:
 *   A pointer to the absolute pathname string. If NULL is returned, there was
 *   an error, and the pathname could not be determined.
 *
 * Original author: Evan Myers
 *****************************************************************************/
char *get_config_path (const char *filename);


#endif //__CONFIG_H__
