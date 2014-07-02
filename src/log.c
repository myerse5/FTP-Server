/******************************************************************************
 * FTP-Server
 * Author: Evan Myers
 * Date: July 2014
 *
 * Description:
 *   Functions that report server usage.
 *****************************************************************************/
#include <stdio.h>
#include <time.h>
#include "log.h"


/******************************************************************************
 * log_received_cmd - see log.h
 *****************************************************************************/
void log_received_cmd (char *user, char *cmd, char *arg, int numArgs)
{
  struct tm *timeInfo;
  time_t rawTime;

  time (&rawTime);
  timeInfo = localtime (&rawTime);

  fprintf (stderr, "%s\tUser <%s>\n", asctime (timeInfo), user);

  if (cmd == NULL) 
    fprintf (stderr, "\tERROR: Missing Command\n");

  fprintf (stderr, "\tEntered Command <%s> with (%d) Argument(s) \"%s\"\n\n",
	   cmd, (numArgs - 1), arg);
}
