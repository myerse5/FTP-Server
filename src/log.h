/******************************************************************************
 * FTP-Server
 * Author: Evan Myers
 * Date: July 2014
 *
 * Description:
 *   Functions that report server usage.
 *****************************************************************************/
#include <time.h>


/******************************************************************************
 * log_received_cmd - see log.h
 *****************************************************************************/
void log_received_cmd (char *user, char *cmd, char *arg, int numArgs);
