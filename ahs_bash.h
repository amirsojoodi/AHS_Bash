/*****************************************************
*                                                    *
*            Simple shell written in C.              *
*       by Amir Hossein Sojoodi, (C) 2012            *
*                                                    *
*                                                    *
******************************************************/

#ifndef AHS_BASH
#define AHS_BASH

#include "ahs_parser.h"
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>


// definitions

typedef enum {RUNNING, STOPPED} TASK_STATE;

typedef struct bgtask{
	int number;
	struct bgtask *previous;
	struct bgtask *next;
	TASK_STATE state;
	char *name;
	pid_t pid;
} BGTASK;

//PROTOTYPE
int local_shell();

int help();

void print_command(COMMAND *command);

void handle_signal(int);

int execute(COMMAND *command);

void change_directory(char *);

void bgtask_list();

void bgtask_kill(int, int);

void add_bgtask(char *, pid_t pid);

void check_bgtasks();

void print_bgtask(BGTASK *);

int get_pid(char *);

BGTASK *get_bgtask_by_number(int);

int is_pid_exist(int);

void initialize();

void set_bash_line();

void deconstruct();

#endif

