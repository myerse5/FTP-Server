/******************************************************************************
 * Authors: Evan Myers, Justin Slind, Alex Tai, James Yoo
 * FTP-Server
 * Date: November 2013
 *
 * Description: 
 *    The functions found in this file send a response message to the client
 *    over the control connection socket. They were created to remove
 *    repetition in places where a response message is sent. Also, this will
 *    help ensure that all messages that are meant to send the same data will
 *    do so.
 *****************************************************************************/
#include <string.h>
#include <stdio.h>
#include "net.h"
#include "reply.h"


/* A response message sent over the control connection should fit on a
 * standard, default terminal line. */
#define STD_TERM_SZ 80  //The standard number of characters in a terminal line.


/******************************************************************************
 * send_welcome_mesg_220 - see "reply.h"
 *****************************************************************************/
int send_welcome_mesg_220 (int c_sfd)
{
  uint8_t mesg[] = "220 FTP server ready.\n";
  int mesg_len;

  //Send the complete response message.
  mesg_len = strlen ((char *)mesg);
  if (send_all (c_sfd, mesg, mesg_len) == -1) {
    return -1;
  }
  
  return 0;
}

/******************************************************************************
 * send_mesg_214_general - see reply.h
 *****************************************************************************/
void send_mesg_214_general (int csfd)
{
  const char *helpMsgStart = "\n214 - Help message.\n",
    *row1 = "The following commands may be abbreviated.\n"
            "The available commands are:\n",
    *row2 = "\tABOR\tHELP\tPASV\tRMD\tSTOU\n",
    *row3 = "\tACCT\tLIST\tPORT\tRNFR\tSTRU\n",
    *row4 = "\tALLO\tMKD\tPWD\tRNTO\tSYST\n",
    *row5 = "\tAPPE\tMODE\tQUIT\tSITE\tTYPE\n",
    *row6 = "\tCDUP\tNLST\tREIN\tSMNT\tUSER\n",
    *row7 = "\tCWD\tNOOP\tREST\tSTAT\n",
    *row8 = "\tDELE\tPASS\tRETR\tSTOR\n",
    *helpMsgEnd = "214 - Help is OK.\n\n";

    send_all (csfd, (uint8_t *)helpMsgStart, strlen (helpMsgStart));
    send_all (csfd, (uint8_t *)row1, strlen (row1));
    send_all (csfd, (uint8_t *)row2, strlen (row2));
    send_all (csfd, (uint8_t *)row3, strlen (row3));
    send_all (csfd, (uint8_t *)row4, strlen (row4));
    send_all (csfd, (uint8_t *)row5, strlen (row5));
    send_all (csfd, (uint8_t *)row6, strlen (row6));
    send_all (csfd, (uint8_t *)row7, strlen (row7));
    send_all (csfd, (uint8_t *)row8, strlen (row8));
    send_all (csfd, (uint8_t *)helpMsgEnd, strlen (helpMsgEnd));
}


/******************************************************************************
 * send_mesg_214_specific - see reply.h
 *****************************************************************************/
void send_mesg_214_specific (int csfd, char *syntax, char *info)
{
  const char *helpMesgEnd = "214 - Help is OK.\n\n";
  const char *helpMesgStart = "\n214 - Help message.\n";

  send_all (csfd, (uint8_t *)helpMesgStart, strlen (helpMesgStart));
  send_all (csfd, (uint8_t *)syntax, strlen (syntax));
  send_all (csfd, (uint8_t *)info, strlen (info));
  send_all (csfd, (uint8_t *)helpMesgEnd, strlen (helpMesgEnd));
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
  int h1, h2, h3, h4;         //The bytes of the IPv4 address
  int p1, p2;                 //The bytes of the port

  /* Initialize addr_len to the size of an IPv4 address. If getsockname()
   * modifies this value, an IPv4 address was not used to create the PASV
   * socket. */
  addrLen = sizeof(sa);

  //Collect the address info
  if (getsockname (dsfd, (struct sockaddr *)&sa, &addrLen) == -1)
    return -1;

  //Ensure the passive socket has a 4-byte address.
  if (addrLen > sizeof(sa)) {
    //The socket is not IPv4, the socket is invalid for the command PASV.
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

  //Create the feedback message, code 227.
  sprintf ((char *)mesg, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d).\n", 
	   h1, h2, h3, h4, p1, p2);

  mesgLen = strlen ((char *)mesg);
  //Send the feedback message to the control socket.
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

  //Send the complete response message.
  mesgLen = strlen ((char *)mesg);  
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
  uint8_t mesg[] = "451 Requested action aborted. Local error in processing.\n";
  int mesgLen;

  //Send the complete response message.
  mesgLen = strlen ((char *)mesg);  
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
  uint8_t mesg[] = "500 Syntax error, command unrecognized.\n";
  int mesgLen;

  //Send the complete response message.
  mesgLen = strlen ((char *)mesg);  
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

  //Send the complete response message.
  mesgLen = strlen ((char *)mesg);
  if (send_all (csfd, mesg, mesgLen) == -1) {
    return -1;
  }
  return 0;
}


/******************************************************************************
 * send_mesg_530 - see "reply.h"
 *****************************************************************************/
int send_mesg_530 (int csfd)
{
  uint8_t mesg[] = "530 Please login with USER and PASS.\n";
  int mesgLen;

  //Send the complete response message.
  mesgLen = strlen ((char *)mesg);
  if (send_all (csfd, mesg, mesgLen) == -1) {
    return -1;
  }
  return 0;
}


/******************************************************************************
 * send_mesg_550 - see "reply.h"
 *****************************************************************************/
int send_mesg_550 (int csfd)
{
  uint8_t mesg[] = "550 Requested action not taken. File unavailable.\n";
  int mesgLen;

  //Send the complete response message.
  mesgLen = strlen ((char *)mesg);
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
  uint8_t mesg[] = "553 Requested action not taken. File name not allowed.\n";
  int mesgLen;

  //Send the complete response message.
  mesgLen = strlen ((char *)mesg);
  if (send_all (csfd, mesg, mesgLen) == -1) {
    return -1;
  }
  return 0;
}
