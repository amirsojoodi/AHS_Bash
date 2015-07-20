/*****************************************************
*                                                    *
*            Simple shell written in C.              *
*       by Amir Hossein Sojoodi, (C) 2012            *
*                                                    *
*                                                    *
******************************************************/

#ifndef AHS_NETSHELL
#define AHS_NETSHELL

#include "ahs_bash.h"
#include "ahs_parser.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <netdb.h>
#include <netinet/in.h>

extern char *CWD;
extern char *CD;
extern char *PWD;
extern char *EXIT;
extern char *HELP;
extern char *CLEAR;
extern char *BGLIST;
extern char *BGSTOP;
extern char *BGKILL;
extern char *BGSTART;
extern char *BASH;
extern char *BASH_LINE;
extern char hostname[100];
extern char *COMMAND_ERROR;
extern char *COMMAND_ERRORC;
extern char *CD_ERROR;

extern BGTASK *bgtask_head, *bgtask_tail;
extern int BGTASK_NUMBER;
extern int BGTASK_COUNTER;
extern int IS_RUNNING_FGTASK;

int server_shell(int, char **);

int client_shell(int, char **);

void netshell(char *, int);

int server_execute(COMMAND *, char *, int);

void server_change_directory(char *, char *, int);

void server_check_bgtasks(char *, int);

void server_bgtask_list(char *, int);

void server_handle_signal(int);

void server_initialize(char *, int);

void server_print_bgtask(BGTASK *, char *, int);

void server_print_command(COMMAND *, char *, int);

void server_help(char *, int);

void server_bgtask_kill(int, int, char *, int);

#define LISTENQ	(2048)
#define PORT (2012)
#define MAX_LEN 1000

#endif


