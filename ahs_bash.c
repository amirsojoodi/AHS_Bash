/*****************************************************
*                                                    *
*            Simple shell written in C.              *
*	by Amir Hossein Sojoodi, (C) 2012            *
*                                                    *
*                                                    *
******************************************************/


#include "ahs_parser.h"
#include "ahs_bash.h"
#include "ahs_netbash.h"


char *CWD;
char *CD = "cd";
char *PWD = "pwd";
char *EXIT = "exit";
char *HELP = "help";
char *CLEAR = "clear";
char *BGLIST = "bglist";
char *BGSTOP = "bgstop";
char *BGKILL = "bgkill";
char *BGSTART = "bgstart";
char *BASH = "ahs_shell># ";
char *BASH_LINE;
char hostname[100] = "\0";
char *COMMAND_ERROR = "Command not found!";
char *COMMAND_ERRORC = "Command not complete!";
char *CD_ERROR = "You don't have permission or there is no directory named =";

BGTASK *bgtask_head, *bgtask_tail;
int BGTASK_NUMBER = 0;
int BGTASK_COUNTER = 0;
int IS_RUNNING_FGTASK = 0;

int main(int argc, char **argv){

	if(argc < 2){
		initialize();
		local_shell();
	}
	else if(!strcmp(argv[1], "--server")){
		server_shell(argc, argv);	
	}
	else if(!strcmp(argv[1], "--client")){
		client_shell(argc, argv);
	}
	else{
		help();
	}
	exit(EXIT_SUCCESS);
}

// local shell 
int local_shell(){
	char *line;
	int flag = 1;
	COMMAND *command;
	help();
	while(flag){
		line = (char *)malloc(100*sizeof(char));
		command = (COMMAND *)malloc(sizeof(int) + sizeof(char **) + sizeof(char *) + sizeof(boolean));
		printf("%s", BASH_LINE);
		//gets(line);
		getline(line, 100);
		if(!strlen(line)){
			//fputs("\033[A",stdout);//\033[A\033[2k
			//rewind(stdout);
			//ftruncate(1,0);
			continue;
		}
		parse(line, command);
		//print_command(command);//for verify the parsing
		if(!strcmp(command->com, EXIT)){
			if(command->argc < 2){
				flag = 0;
			}
			else{
				printf("%s\n", COMMAND_ERROR);
			}
		}
		else if(!strcmp(command->com, HELP)){
			if(command->argc < 2){
				help();
			}
			else{
				printf("%s\n", COMMAND_ERROR);
			}
		}
		else if(!strcmp(command->com, CLEAR)){
			if(command->argc < 2){
				system(CLEAR);
			}
			else{
				printf("%s\n", COMMAND_ERROR);
			}
		}
		else if(!strcmp(command->com, PWD)){
			if(command->argc > 1){
				printf("%s\n", COMMAND_ERROR);
			}
			else{
				printf("%s\n", CWD);
			}
		}
		else if(!strcmp(command->com, CD)){
			if(command->argc > 2){
				printf("%s\n", COMMAND_ERROR);
			}
			else if(command->argc == 1){
				change_directory(getenv("HOME"));		
			}
			else if(!strcmp(command->argv[1], "..")){
				change_directory(get_parent_directory(CWD));
			}
			else if(!strcmp(command->argv[1], ".")){
				change_directory(CWD);
			}
			else{
				change_directory(command->argv[1]);
			}
		}
		else if(!strcmp(command->com, BGLIST)){
			if(command->argc > 1){
				printf("%s\n", COMMAND_ERROR);	
			}
			else{
				bgtask_list();
			}
		}
		else if(!strcmp(command->com, BGSTART)){
			if(command->argc < 2){
				printf("%s\n", COMMAND_ERRORC);	
			}
			else{
				bgtask_kill(atoi(command->argv[1]), SIGCONT);
			}
		}
		else if(!strcmp(command->com, BGSTOP)){
			if(command->argc < 2){
				printf("%s\n", COMMAND_ERRORC);	
			}
			else{
				bgtask_kill(atoi(command->argv[1]), SIGSTOP);
			}
		}
		else if(!strcmp(command->com, BGKILL)){
			if(command->argc < 2){
				printf("%s\n", COMMAND_ERRORC);	
			}
			else{
				bgtask_kill(atoi(command->argv[1]), SIGTERM);
			}
		}
		else{
			if(!strcmp(command->com, "bg") && command->argc < 2){
				printf("%s\n", COMMAND_ERRORC);
			}	
			else{
				execute(command);
			}
		}
		check_bgtasks();
		free(line);
		free(command);
	}
	deconstruct();
	exit(EXIT_SUCCESS);
}

// execute the arbitrary command
int execute(COMMAND *command){
	pid_t pid = fork();
	
	if(pid == 0){	//child
		if(command->bg){
			signal(SIGINT, handle_signal);
			signal(SIGUSR1, handle_signal);
		}
		execvp(command->com, command->argv);
		printf("%s: %s\n", command->com, COMMAND_ERROR);
		exit(EXIT_FAILURE);
	}
	else if(pid > 0){ //parent
		int childStatus;
		if(!command->bg){
			IS_RUNNING_FGTASK = 1;
			waitpid(pid, &childStatus, WUNTRACED);
			waitpid(pid, &childStatus, WUNTRACED);
		}
		else{
			sleep(1);
			if(waitpid(pid, &childStatus, WNOHANG) == 0){
			        int new_pid = get_pid(command->com);
				add_bgtask(command->com, new_pid);
			}
		}
		IS_RUNNING_FGTASK = 0;
	}
	else{
		printf("CHILD_PROCESS creation failed!");
	}
	return 0;
}

//change directory
void change_directory(char *dir){
	int status = chdir(dir);
	if(status >= 0){
		set_bash_line();
	} else {
		printf("%s %s\n", CD_ERROR, dir);
	}
	return;
}

// check background tasks
void check_bgtasks(){
	if(BGTASK_NUMBER == 0)
		return;
	
	BGTASK *tmp = bgtask_head;
	int terminated = 0;
	int i = 0;
	for(; i < BGTASK_NUMBER; i++){
		//print_bgtask(tmp);
		int status;
		if(waitpid(tmp->pid, &status, WNOHANG) != 0){ //process is gone
			terminated++;
			printf(" (%d) %s is terminated.\n", tmp->number, tmp->name);
			if(tmp->previous){
				tmp->previous->next = tmp->next;
			}
			else{
				bgtask_head = tmp->next;
			}
			if(tmp->next){
				tmp->next->previous = tmp->previous;
			}
		}
		tmp = tmp->next;
	}
	BGTASK_NUMBER -= terminated;
	return;
}

//background task list
void bgtask_list(){
	check_bgtasks();
	if(BGTASK_NUMBER == 0){
		printf("There isn't any background task!\n");
		return;
	}
	int i = 0;
	BGTASK *tmp = bgtask_head;
	for(; i < BGTASK_NUMBER; i++){
		char *state = (tmp->state == RUNNING)? "running.":"stopped.";
		printf(" (%d) %s is %s\n",tmp->number, tmp->name, state);
		tmp = tmp->next;
	}
	printf("\nTotal background jobs: %d\n", BGTASK_NUMBER);
	return;
}

//new background task
void add_bgtask(char *name, pid_t pid){
	int size = sizeof(int) + sizeof(TASK_STATE) + sizeof(pid_t);
	BGTASK *tmp = (BGTASK *)malloc(2 * sizeof(BGTASK *) + sizeof(char *) + size);
	tmp->name = (char *)malloc(strlen(name) * sizeof(char));
	strcpy(tmp->name, name);
	tmp->number = ++BGTASK_COUNTER;
	tmp->state = RUNNING;
	tmp->pid = pid;
	if(bgtask_head == NULL){
		bgtask_head = tmp;
	}
	else {	
		BGTASK *iterator = bgtask_head;
		int i = 1;
		for(; i < BGTASK_NUMBER; i++){
			iterator = iterator->next;
		}
		iterator->next = tmp;
		tmp->previous = iterator;
	}
	BGTASK_NUMBER++;
	return;
}

// get the pid of running process!
int get_pid(char *name){
	char *buf = (char *)malloc((18 + strlen(name))*sizeof(char));
	sprintf(buf, "/bin/ps -o pid -C %s", name);
	FILE *fp = popen(buf, "r");
  	char path[1035];

	if (fp == NULL) {
		printf("Failed to run ps command..\n" );
		deconstruct();
		exit(0);
	}
	int pid;
	fgets(path, sizeof(path) - 1, fp);
	while (fgets(path, sizeof(path) - 1, fp) != NULL) {
		sscanf(path, "%d", &pid);
		//printf("Searching for the pid = %d\n", pid);
		if(!is_pid_exist(pid)){
			pclose(fp);
			return pid;
		}
	}
	pclose(fp);
	return -1;
}

// bgtask kill with signals TERM, CONT, STOP 
void bgtask_kill(int number, int signal){
	BGTASK *tmp = get_bgtask_by_number(number);
	if(tmp == NULL){
		printf("There is no process (%d)\n", number);
		return;
	}
	kill(tmp->pid, signal);
	if(signal == SIGCONT){
		tmp->state = RUNNING;
	}
	else{
		tmp->state = STOPPED;
	}
	sleep(1);
	return;
}

// get pid by number
BGTASK *get_bgtask_by_number(int number){
	if(BGTASK_NUMBER == 0){
		return NULL;
	}
	int i = 0;
	BGTASK *tmp = bgtask_head;
	for(; i < BGTASK_NUMBER; i++){
		if(tmp->number == number){
			return tmp;
		}
		tmp = tmp->next;
	}
	return NULL;
}
	
// search for pid in the queue
int is_pid_exist(int pid){
	if(BGTASK_NUMBER == 0){
		return 0;
	}
	int i = 0;
	BGTASK *tmp = bgtask_head;
	for(; i < BGTASK_NUMBER; i++){
		if(tmp->pid == pid){
			return 1;
		}
		tmp = tmp->next;
	}
	return 0;
}

//print the bash help
int help(){
	printf("\n.................. Welcome to AHS_SHELL ..................\n\n");
	printf("Runnig options:\n");
	printf("\t --server IP[defult-localhost] PORT[defult-2012]\n");
	printf("\t\tto run the server side of the shell.\n");
	printf("\t --client IP PORT\n");
	printf("\t\tto run the client side of the shell.\n");
	printf("\t without any option\n");
	printf("\t\tto run the shell locally.\n");
	printf("\nThe commands you can type inside the shell:\n");
	printf("\thelp -> to see this.\n");
	printf("\tclear-> to clear the screen.\n");
	printf("\texit -> to exit from the shell.\n");
	printf("\tand any valid command on linux!\n");
	printf("\n\t\t     --> Have fun! <--\n\n");
	printf("....................... AHS_SHELL .......................\n\n");
	return 0;
}

//print the input command in detail
void print_command(COMMAND *command){
	int argc = command->argc;
	printf("command argc = %d\n", command->argc);
	char **tmp = command->argv;
	while(argc--){
		printf("%s\n", *tmp++);
	}
}

//print bgtask in detail
void print_bgtask(BGTASK *tmp){
	printf("number = %d, pid = %d, name = %s\n", tmp->number, tmp->pid, tmp->name);
	return;
}

//initial the bash
void initialize(){
	//signal(SIGINT, SIG_IGN);
	signal(SIGINT, handle_signal);	
	CWD = (char *)malloc(100 * sizeof(char));
	BASH_LINE = (char *)malloc(100 * sizeof(char));
	gethostname(hostname, 20);
	system(CLEAR);
	set_bash_line();
	return;
}

//set BASH_LINE
void set_bash_line(){
	getcwd(CWD, 100);
	strcpy(BASH_LINE, getenv("USER"));
	strcat(BASH_LINE, "@");
	strcat(BASH_LINE, hostname);
	strcat(BASH_LINE, ":");
	strcat(BASH_LINE, CWD);
	if(strlen(CWD) != 1)
		strcat(BASH_LINE,"/");
	strcat(BASH_LINE, BASH);
	return;
}

// handle ctrl+c signal
void handle_signal(int signo)
{
	signal(SIGINT, handle_signal);
	switch(signo){
	case (SIGINT):
		if(!IS_RUNNING_FGTASK){
			printf("\n%s", BASH_LINE);
			fflush(stdout);
		}
		else {
			fflush(stdout);
			printf("\n");
		}
		break;

	case (SIGUSR1):
		fflush(stdout);
		break;
	}
	return;
}

//deconstruct
void deconstruct(){
	free(CWD);
	free(BASH_LINE);
	return;
}
