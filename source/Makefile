###############################################################################
# Students: Evan Myers, Justin Slind, Alex Tai, James Yoo
# Course: CMPT-361
# Assignment #3 - ftp server
# File: Makefile
# Date: November 2013
###############################################################################
CC	=	gcc
CFLAGS	=	-g -pedantic -pthread -std=c99 -Wall -D_BSD_SOURCE -D_POSIX_C_SOURCE=200112L -D_GNU_SOURCE
LDFLAGS	=	-pthread


#main program
ftpd: 	config.o ctrlthread.o directory.o help.o main.o md5.o misc.o net.o parser.o path.o queue.o reply.o servercmd.o session.o switch.o transfer.o users.o
	$(CC) $(LDFLAGS) -o ftpd $^


#components
config.o:	config.c config.h

ctrlthread.o:	ctrlthread.c ctrlthread.h reply.h session.h

directory.o: 	directory.c directory.h net.h path.h reply.h session.h

help.o:		help.c help.h net.h session.h

main.o:		main.c config.h ctrlthread.h net.h servercmd.h

md5.o:		md5.c common.h md5.h

misc.o: 	misc.c misc.h net.h session.h

net.o:		net.c config.h net.h reply.h session.h

parser.o: 	parser.c parser.h

path.o:		path.c path.h reply.h session.h

queue.o:	queue.c queue.h

reply.o:	reply.c net.h reply.h

servercmd.o:	servercmd.c config.h ctrlthread.h net.h servercmd.h

session.o:	session.c net.h session.h switch.h queue.h

switch.o: 	switch.c directory.h help.h misc.h net.h parser.h session.h switch.h transfer.h users.h

transfer.o: 	transfer.c net.h path.h reply.h session.h transfer.h

users.o:	users.c config.h md5.h net.h session.h users.h


#Clean up the repository.
.PHONY:	clean
clean:
	$(RM) ftpd config.o ctrlthread.o directory.o help.o main.o md5.o misc.o net.o parser.o path.o queue.o reply.o servercmd.o session.o switch.o transfer.o users.o
