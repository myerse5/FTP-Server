/******************************************************************************
 * Students: Evan Myers, Justin Slind, Alex Tai, James Yoo
 * Course: CMPT-361
 * Assignment #3 - ftp server
 * File: reply.c
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
 * send_mesg_227 - see "reply.h"
 *****************************************************************************/
int send_mesg_227 (int c_sfd, int d_sfd)
{
  struct sockaddr_in my_addr;
  socklen_t addr_len;
  uint8_t mesg[STD_TERM_SZ];
  int mesg_len;

  /* Used to collect the the decimal value of each byte, which will be sent to
   * the client as specified RFC 959. */
  int h1, h2, h3, h4;         //The bytes of the IPv4 address
  int p1, p2;                 //The bytes of the port

  /* Initialize addr_len to the size of an IPv4 address. If getsockname()
   * modifies this value, an IPv4 address was not used to create the PASV
   * socket. */
  addr_len = sizeof(my_addr);

  //Collect the address info
  if (getsockname (d_sfd, (struct sockaddr *)&my_addr, &addr_len) == -1)
    return -1;

  //Ensure the passive socket has a 4-byte address.
  if (addr_len > sizeof(my_addr)) {
    //The socket is not IPv4, the socket is invalid for the command PASV.
    return -1;
  }

  /* Ensure the IPv4 address bytes will be sent to the client in the same order
   * on all systems by converting to network-byte-order before calculating
   * the value of each byte field. */
  my_addr.sin_addr.s_addr = htonl (my_addr.sin_addr.s_addr);
  /* Store each byte of the IPv4 address as a decimal value in preparation for
   * sending the passive mode message. */
  h1 = (my_addr.sin_addr.s_addr & 0xFF000000) >> (3 * BITS_IN_BYTE);
  h2 = (my_addr.sin_addr.s_addr & 0x00FF0000) >> (2 * BITS_IN_BYTE);
  h3 = (my_addr.sin_addr.s_addr & 0x0000FF00) >> (BITS_IN_BYTE);
  h4 = (my_addr.sin_addr.s_addr & 0x000000FF);

  /* Ensure the port bytes will be sent to the client in the same order
   * on all systems by converting to network-byte-order before calculating
   * the value of each byte field. */
  my_addr.sin_port = htons (my_addr.sin_port);
  /* Store each byte of the port as a decimal value in preparation for
   * sending the passive mode message. */
  p1 = (my_addr.sin_port & 0xFF00) >> BITS_IN_BYTE;
  p2 = (my_addr.sin_port & 0x00FF);

  //Create the feedback message, code 227.
  sprintf ((char *)mesg, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d).\n", 
	   h1, h2, h3, h4, p1, p2);

  mesg_len = strlen ((char *)mesg);
  //Send the feedback message to the control socket.
  if (send_all (c_sfd, mesg, mesg_len) == -1) {
    return -1;
  }

  return 0;
}


/******************************************************************************
 * send_mesg_450 - see "reply.h"
 *****************************************************************************/
int send_mesg_450 (int c_sfd)
{
  uint8_t mesg[] = "450 Requested file action not taken. File unavailable.\n";
  int mesg_len;

  //Send the complete response message.
  mesg_len = strlen ((char *)mesg);  
  if (send_all (c_sfd, mesg, mesg_len) == -1) {
    return -1;
  }

  return 0;
}


/******************************************************************************
 * send_mesg_451 - see "reply.h"
 *****************************************************************************/
int send_mesg_451 (int c_sfd)
{
  uint8_t mesg[] = "451 Requested action aborted. Local error in processing.\n";
  int mesg_len;

  //Send the complete response message.
  mesg_len = strlen ((char *)mesg);  
  if (send_all (c_sfd, mesg, mesg_len) == -1) {
    return -1;
  }


  return 0;
}


/******************************************************************************
 * send_mesg_500 - see "reply.h"
 *****************************************************************************/
int send_mesg_500 (int c_sfd)
{
  uint8_t mesg[] = "500 Syntax error, command unrecognized.\n";
  int mesg_len;

  //Send the complete response message.
  mesg_len = strlen ((char *)mesg);  
  if (send_all (c_sfd, mesg, mesg_len) == -1) {
    return -1;
  }

  return 0;
}


/******************************************************************************
 * send_mesg_501 - see "reply.h"
 *****************************************************************************/
int send_mesg_501 (int c_sfd)
{
  uint8_t mesg[] = "501 Syntax error in parameters or arguments.\n";
  int mesg_len;

  //Send the complete response message.
  mesg_len = strlen ((char *)mesg);
  if (send_all (c_sfd, mesg, mesg_len) == -1) {
    return -1;
  }

  return 0;
}


/******************************************************************************
 * send_mesg_530 - see "reply.h"
 *****************************************************************************/
int send_mesg_530 (int c_sfd)
{
  uint8_t mesg[] = "530 Please login with USER and PASS.\n";
  int mesg_len;

  //Send the complete response message.
  mesg_len = strlen ((char *)mesg);
  if (send_all (c_sfd, mesg, mesg_len) == -1) {
    return -1;
  }

  return 0;
}


/******************************************************************************
 * send_mesg_550 - see "reply.h"
 *****************************************************************************/
int send_mesg_550 (int c_sfd)
{
  uint8_t mesg[] = "550 Requested action not taken. File unavailable.\n";
  int mesg_len;

  //Send the complete response message.
  mesg_len = strlen ((char *)mesg);
  if (send_all (c_sfd, mesg, mesg_len) == -1) {
    return -1;
  }
  return 0;
}


/******************************************************************************
 * send_mesg_553 - see "reply.h"
 *****************************************************************************/
int send_mesg_553 (int c_sfd)
{
  uint8_t mesg[] = "553 Requested action not taken. File name not allowed.\n";
  int mesg_len;

  //Send the complete response message.
  mesg_len = strlen ((char *)mesg);
  if (send_all (c_sfd, mesg, mesg_len) == -1) {
    return -1;
  }
  return 0;
}
