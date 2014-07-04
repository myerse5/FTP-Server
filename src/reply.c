/******************************************************************************
 * FTP-Server
 * Authors: Evan Myers, Justin Slind, Alex Tai, James Yoo
 * Date: November 2013
 *
 * Description: 
 *    All response messages sent to the client can be found in this file. All
 *    communication sent by the server to the client on the control connection
 *    is sent by a function found in this file.
 *
 *    This has been done to ensure all communication is consistent, and to
 *    make modififying these responses easier.
 *****************************************************************************/
#include <string.h>
#include <stdio.h>
#include "net.h"
#include "reply.h"


/* A response message sent over the control connection should fit on a
 * standard, default terminal line. */
#define STD_TERM_SZ 80  // Use for one line replies where length is not known.


/******************************************************************************
 * send_mesg_150 - see "reply.h"
 *****************************************************************************/
int send_mesg_150 (int csfd, const char *filename, char option)
{
  char *reply;
  int mesgLen;

  if (option == REPLY_150_ASCII) {
    reply = "150 Opening ASCII mode data connection for ";
  } else if (option == REPLY_150_BINARY) {
    reply = "150 Opening BINARY mode data connection for ";
  }

  mesgLen = strlen (reply);
  if (send_all (csfd, (uint8_t*)reply, mesgLen) == -1)
    return -1;

  mesgLen = strlen (filename);
  if (send_all (csfd, (uint8_t*)filename, mesgLen) == -1)
    return -1;

  reply = "\n";
  mesgLen = strlen (reply);
  if (send_all (csfd, (uint8_t*)reply, mesgLen) == -1)
    return -1;

  return 0;
}


/******************************************************************************
 * send_mesg_200 - see "reply.h"
 *****************************************************************************/
int send_mesg_200 (int csfd, char option)
{
  char *reply;
  int mesgLen;

  if (option == REPLY_200_PORT) {
    reply = "200 PORT command successful. Consider using PASV.\n";
  } else if (option == REPLY_200_ASCII) {
    reply = "200 Switching to ASCII mode.\n";
  } else if (option == REPLY_200_IMAGE) {
    reply = "200 Switching to Image mode.\n";
  } else if (option == REPLY_200_STREAM) {
    reply = "200 Switching to stream mode.\n";
  } else if (option == REPLY_200_FSTRU) {
    reply = "200 Switching to File Structure.\n";
  }

  mesgLen = strlen (reply);
  if (send_all (csfd, (uint8_t*)reply, mesgLen) == -1) {
    return -1;
  }
  return 0;
}


/******************************************************************************
 * send_mesg_215 - see "reply.h"
 *****************************************************************************/
int send_mesg_215 (int csfd)
{
  uint8_t mesg[] = "215 UNIX Type: L8\n";
  int mesgLen;

  mesgLen = strlen ((char*)mesg);
  if (send_all (csfd, mesg, mesgLen) == -1) {
    return -1;
  }
  return 0;
}


/******************************************************************************
 * send_mesg_220 - see "reply.h"
 *****************************************************************************/
int send_mesg_220 (int csfd)
{
  uint8_t mesg[] = "220 FTP server ready.\n";
  int mesgLen;

  mesgLen = strlen ((char*)mesg);
  if (send_all (csfd, mesg, mesgLen) == -1)
    return -1;

  return 0;
}

/******************************************************************************
 * send_mesg_214_general - see reply.h
 *****************************************************************************/
int send_mesg_214_general (int csfd)
{
  int mesgLen;
  uint8_t mesg[] =
    "214-The following commands are recognized.\n"
    " APPE CDUP CWD  HELP LIST MKD  MODE NLST PASS PASV PORT PWD  QUIT RETR\n"
    " STOR STOU STRU SYST TYPE USER\n"
    "214 Help OK.\n";

  mesgLen = strlen ((char*)mesg);
  if (send_all (csfd, mesg, mesgLen) == -1) {
    return -1;
  }
  return 0;
}


/******************************************************************************
 * send_mesg_214_specific - see reply.h
 *****************************************************************************/
int send_mesg_214_specific (int csfd, char *syntax, char *info)
{
  char *reply;
  int mesgLen;

  reply = "214-Help message.\n";
  mesgLen = strlen (reply);
  if (send_all (csfd, (uint8_t*)reply, mesgLen) == -1)
    return -1;

  if (send_all (csfd, (uint8_t*)syntax, strlen (syntax)) == -1)
    return -1;

  if (send_all (csfd, (uint8_t*)info, strlen (info)) == -1)
    return -1;

  reply = "214 Help is OK.\n";
  mesgLen = strlen (reply);
  if (send_all (csfd, (uint8_t*)reply, strlen (reply)) == -1) {
    return -1;
  }
  return 0;
}


/******************************************************************************
 * send_mesg_221 - see "reply.h"
 *****************************************************************************/
int send_mesg_221 (int csfd)
{
  uint8_t mesg[] = "221 Quitting system; goodbye.\n";
  int mesgLen;

  mesgLen = strlen ((char*)mesg);
  if (send_all (csfd, mesg, mesgLen) == -1) {
    return -1;
  }
  return 0;
}


/******************************************************************************
 * send_mesg_226 - see "reply.h"
 *****************************************************************************/
int send_mesg_226 (int csfd, char option)
{
  char *reply;
  int mesgLen;

  if (option == REPLY_226_ABORT) {
    reply = "226 Abort successful.\n";
  } else if (option == REPLY_226_SUCCESS) {
    reply = "226 Closing data connection; requested file action successful.\n";
  }

  mesgLen = strlen (reply);  
  if (send_all (csfd, (uint8_t*)reply, mesgLen) == -1) {
    return -1;
  }
  return 0;
}


/******************************************************************************
 * send_mesg_227 - see "reply.h"
 *****************************************************************************/
int send_mesg_227 (int csfd, int dsfd)
{
  struct sockaddr_in sa;
  socklen_t addrLen;
  uint8_t mesg[STD_TERM_SZ];
  int mesgLen;

  /* Used to collect the the decimal value of each byte, which will be sent to
   * the client as specified RFC 959. */
  int h1, h2, h3, h4;         // The bytes of the IPv4 address
  int p1, p2;                 // The bytes of the port

  /* Initialize addr_len to the size of an IPv4 address. If getsockname()
   * modifies this value, an IPv4 address was not used to create the PASV
   * socket. */
  addrLen = sizeof(sa);

  // Collect the address info
  if (getsockname (dsfd, (struct sockaddr *)&sa, &addrLen) == -1)
    return -1;

  // Ensure the passive socket has a 4-byte address.
  if (addrLen > sizeof(sa)) {
    // The socket is not IPv4, the socket is invalid for the command PASV.
    return -1;
  }

  /* Ensure the IPv4 address bytes will be sent to the client in the same order
   * on all systems by converting to network-byte-order before calculating
   * the value of each byte field. */
  sa.sin_addr.s_addr = htonl (sa.sin_addr.s_addr);
  /* Store each byte of the IPv4 address as a decimal value in preparation for
   * sending the passive mode message. */
  h1 = (sa.sin_addr.s_addr & 0xFF000000) >> (3 * BITS_IN_BYTE);
  h2 = (sa.sin_addr.s_addr & 0x00FF0000) >> (2 * BITS_IN_BYTE);
  h3 = (sa.sin_addr.s_addr & 0x0000FF00) >> (BITS_IN_BYTE);
  h4 = (sa.sin_addr.s_addr & 0x000000FF);

  /* Ensure the port bytes will be sent to the client in the same order
   * on all systems by converting to network-byte-order before calculating
   * the value of each byte field. */
  sa.sin_port = htons (sa.sin_port);
  /* Store each byte of the port as a decimal value in preparation for
   * sending the passive mode message. */
  p1 = (sa.sin_port & 0xFF00) >> BITS_IN_BYTE;
  p2 = (sa.sin_port & 0x00FF);

  // Create the feedback message, code 227.
  sprintf ((char*)mesg, "227 Entering passive mode (%d,%d,%d,%d,%d,%d).\n", 
	   h1, h2, h3, h4, p1, p2);

  mesgLen = strlen ((char*)mesg);
  // Send the feedback message to the control socket.
  if (send_all (csfd, mesg, mesgLen) == -1) {
    return -1;
  }
  return 0;
}


/******************************************************************************
 * send_mesg_230 - see "reply.h"
 *****************************************************************************/
int send_mesg_230 (int csfd, char option)
{
  char *reply;
  int mesgLen;

  if (option == REPLY_230_NONEED) {
    reply = "230 Already logged in.\n";
  } else if (option == REPLY_230_SUCCESS) {
    reply = "230 Login successful.\n";
  }

  mesgLen = strlen (reply);
  if (send_all (csfd, (uint8_t*)reply, mesgLen) == -1) {
    return -1;
  }
  return 0;
}


/******************************************************************************
 * send_mesg_250 - see "reply.h"
 *****************************************************************************/
int send_mesg_250 (int csfd)
{
  uint8_t mesg[] = "250 Working directory changed.\n";
  int mesgLen;

  mesgLen = strlen ((char*)mesg);  
  if (send_all (csfd, mesg, mesgLen) == -1) {
    return -1;
  }
  return 0;
}


/******************************************************************************
 * send_mesg_257 - see "reply.h"
 *****************************************************************************/
int send_mesg_257 (int csfd, const char *directory)
{
  uint8_t mesgStart[] = "257 \"";
  uint8_t mesgEnd[] = "\"\n";
  int mesgLen;

  mesgLen = strlen ((char*)mesgStart);  
  if (send_all (csfd, mesgStart, mesgLen) == -1) {
    return -1;
  }

  mesgLen = strlen (directory);
  if (send_all (csfd, (uint8_t*)directory, mesgLen) == -1) {
    return -1;
  }

  mesgLen = strlen ((char*)mesgEnd);
  if (send_all (csfd, mesgEnd, mesgLen) == -1) {
    return -1;
  }
  return 0;
}


/******************************************************************************
 * send_mesg_331 - see "reply.h"
 *****************************************************************************/
int send_mesg_331 (int csfd)
{
  uint8_t mesg[] = "331 User name okay, need password.\n";
  int mesgLen;

  mesgLen = strlen ((char*)mesg);  
  if (send_all (csfd, mesg, mesgLen) == -1) {
    return -1;
  }
  return 0;
}


/******************************************************************************
 * send_mesg_425 - see "reply.h"
 *****************************************************************************/
int send_mesg_425 (int csfd)
{
  uint8_t mesg[] = "425 Cannot open data connection; please use the PORT or "
                   "PASV command first.\n";
  int mesgLen;

  mesgLen = strlen ((char*)mesg);  
  if (send_all (csfd, mesg, mesgLen) == -1) {
    return -1;
  }
  return 0;
}


/******************************************************************************
 * send_mesg_426 - see "reply.h"
 *****************************************************************************/
int send_mesg_426 (int csfd)
{
  uint8_t mesg[] = "426 Connection close; transfer aborted.\n";
  int mesgLen;

  mesgLen = strlen ((char*)mesg);  
  if (send_all (csfd, mesg, mesgLen) == -1) {
    return -1;
  }
  return 0;
}

/******************************************************************************
 * send_mesg_450 - see "reply.h"
 *****************************************************************************/
int send_mesg_450 (int csfd)
{
  uint8_t mesg[] = "450 Requested file action not taken. File unavailable.\n";
  int mesgLen;

  mesgLen = strlen ((char*)mesg);  
  if (send_all (csfd, mesg, mesgLen) == -1) {
    return -1;
  }
  return 0;
}


/******************************************************************************
 * send_mesg_451 - see "reply.h"
 *****************************************************************************/
int send_mesg_451 (int csfd)
{
  uint8_t mesg[] = "451 Requested action aborted; local error in processing.\n";
  int mesgLen;

  mesgLen = strlen ((char*)mesg);  
  if (send_all (csfd, mesg, mesgLen) == -1) {
    return -1;
  }
  return 0;
}


/******************************************************************************
 * send_mesg_500 - see "reply.h"
 *****************************************************************************/
int send_mesg_500 (int csfd)
{
  uint8_t mesg[] = "500 Syntax error; command unrecognized.\n";
  int mesgLen;

  mesgLen = strlen ((char*)mesg);  
  if (send_all (csfd, mesg, mesgLen) == -1) {
    return -1;
  }
  return 0;
}


/******************************************************************************
 * send_mesg_501 - see "reply.h"
 *****************************************************************************/
int send_mesg_501 (int csfd)
{
  uint8_t mesg[] = "501 Syntax error in parameters or arguments.\n";
  int mesgLen;

  mesgLen = strlen ((char*)mesg);
  if (send_all (csfd, mesg, mesgLen) == -1) {
    return -1;
  }
  return 0;
}


/******************************************************************************
 * send_mesg_503 - see "reply.h"
 *****************************************************************************/
int send_mesg_503 (int csfd)
{
  uint8_t mesg[] = "503 Login with USER first.\n";
  int mesgLen;

  mesgLen = strlen ((char*)mesg);
  if (send_all (csfd, mesg, mesgLen) == -1) {
    return -1;
  }
  return 0;
}


/******************************************************************************
 * send_mesg_504 - see "reply.h"
 *****************************************************************************/
int send_mesg_504 (int csfd)
{
  uint8_t mesg[] = "504 Command is not currently implemented.\n";
  int mesgLen;

  mesgLen = strlen ((char*)mesg);
  if (send_all (csfd, mesg, mesgLen) == -1) {
    return -1;
  }
  return 0;
}


/******************************************************************************
 * send_mesg_530 - see "reply.h"
 *****************************************************************************/
int send_mesg_530 (int csfd, char option)
{
  char *reply;
  int mesgLen;

  if (option == REPLY_530_REQUEST) {
    reply = "530 Please login with USER and PASS.\n";
  } else if (option == REPLY_530_FAIL) {
    reply = "530 Not logged in.\n";
  }

  mesgLen = strlen (reply);
  if (send_all (csfd, (uint8_t*)reply, mesgLen) == -1) {
    return -1;
  }

  return 0;
}


/******************************************************************************
 * send_mesg_550_no_argument - see "reply.h"
 *****************************************************************************/
int send_mesg_550_no_argument (int csfd)
{
  uint8_t mesg[] = "550 No argument allowed.\n";
  int mesgLen;

  mesgLen = strlen ((char*)mesg);
  if (send_all (csfd, mesg, mesgLen) == -1) {
    return -1;
  }
  return 0;
}


/******************************************************************************
 * send_mesg_550_no_dir - see "reply.h"
 *****************************************************************************/
int send_mesg_550_no_dir (int csfd)
{
  uint8_t mesg[] = "550 Directory not found.\n";
  int mesgLen;

  mesgLen = strlen ((char*)mesg);
  if (send_all (csfd, mesg, mesgLen) == -1) {
    return -1;
  }
  return 0;
}


/******************************************************************************
 * send_mesg_550_process_error - see "reply.h"
 *****************************************************************************/
int send_mesg_550_process_error (int csfd)
{
  uint8_t mesg[] = "550 Error while processing.\n";
  int mesgLen;

  mesgLen = strlen ((char*)mesg);
  if (send_all (csfd, mesg, mesgLen) == -1) {
    return -1;
  }
  return 0;
}


/******************************************************************************
 * send_mesg_550_unavailable - see "reply.h"
 *****************************************************************************/
int send_mesg_550_unavailable (int csfd)
{
  uint8_t mesg[] = "550 Requested action not taken; file unavailable.\n";
  int mesgLen;

  mesgLen = strlen ((char*)mesg);
  if (send_all (csfd, mesg, mesgLen) == -1) {
    return -1;
  }
  return 0;
}


/******************************************************************************
 * send_mesg_553 - see "reply.h"
 *****************************************************************************/
int send_mesg_553 (int csfd)
{
  uint8_t mesg[] = "553 Requested action not taken; file name not allowed.\n";
  int mesgLen;

  mesgLen = strlen ((char*)mesg);
  if (send_all (csfd, mesg, mesgLen) == -1) {
    return -1;
  }
  return 0;
}
