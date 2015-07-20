/*****************************************************
*                                                    *
*            Simple shell written in C.              *
*       by Amir Hossein Sojoodi, (C) 2012            *
*                                                    *
*                                                    *
******************************************************/

#ifndef AHS_PARSER
#define AHS_PARSER

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>

//structures and static variables

typedef enum { false, true } boolean;

typedef struct command {
	int argc;
	char **argv;
	char *com;
	boolean bg;
} COMMAND;


// PROTOTYPES

void parse(char*, COMMAND*);

int ahs_strtok(char *, char *, char **, int);

char *get_parent_directory(char *);

ssize_t readline(int, void *, size_t);

ssize_t writeline(int, const void *, size_t);

void get_line(char *, int);

#endif
