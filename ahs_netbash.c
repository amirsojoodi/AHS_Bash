/*****************************************************
*                                                    *
*            Simple shell written in C.              *
*       by Amir Hossein Sojoodi, (C) 2012            *
*                                                    *
*                                                    *
******************************************************/

#include "ahs_netbash.h"

char *BUFFER;
int CONN;
char *END_OF_MESSAGE = "$";

int server_shell(int argc, char **argv){
	int list_s;
        int conn_s;
	int port;
        struct sockaddr_in servaddr; 
	char buffer[MAX_LEN];
        char *endptr;
        pid_t pid;
	struct hostent *he;
	struct in_addr address;

	if (argc ==  3) {
		port = strtol(argv[2], &endptr, 0);
		if (*endptr) {
			printf("AHS_SERVER_SHELL: Invalid port number.\n");
			exit(EXIT_FAILURE);
		}
	}
	else if(argc == 4){
		port = strtol(argv[3], &endptr, 0);
		if (*endptr) {
			printf("AHS_SERVER_SHELL: Invalid port number.\n");
			exit(EXIT_FAILURE);
		}
		he = gethostbyname(argv[2]);
		if(he){
			bcopy(*he->h_addr_list, (char *)&address, sizeof(address));
		}
		else{
			herror("AHS_SERVER_SHELL");
			exit(EXIT_FAILURE);
		}
	}
	else if(argc < 3) {
		port = PORT;
	}
	else {
		printf("AHS_SERVER_SHELL: Invalid arguments.\n");
		printf("Usage: ./ahs_shell --server Host(defult localhost) Port(defult 2012)\n");
		exit(EXIT_FAILURE);
    	}

    	if ( (list_s = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
		printf("AHS_SERVER_SHELL: Error creating listening socket.\n");
		exit(EXIT_FAILURE);
    	}
        //name->sin_addr = *(struct in_addr *) hostinfo->h_addr;
    	memset(&servaddr, 0, sizeof(servaddr));
    	servaddr.sin_family = AF_INET;
    	//servaddr.sin_addr.s_addr = inet_addr(inet_ntoa(address));//inet_addr(address);
	//servaddr.sin_addr = (struct in_addr)(address.h_addr);
	servaddr.sin_addr = address;
    	servaddr.sin_port = htons(port);


    	if(bind(list_s, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
		printf("AHS_SERVER_SHELL: Error calling bind() on %s:%d\n", inet_ntoa(address), port);
		exit(EXIT_FAILURE);
    	}

	if(listen(list_s, LISTENQ) < 0 ) {
		printf("AHS_SERVER_SHELL: Error calling listen()\n");
		exit(EXIT_FAILURE);
    	}
	
	printf("AHS_SERVER_SHELL: Listening on port: %d\n", port);
        while(1) {
	
		if((conn_s = accept(list_s, NULL, NULL)) < 0){
	    		printf("AHS_SERVER_SHELL: Error calling accept()\n");
	    		exit(EXIT_FAILURE);
		}
		
		printf("A client has connected!\n");
		if((pid = fork()) == 0){
			if ( close(list_s) < 0 ) {
	    			printf("AHS_SERVER_SHELL:Error calling close(list_s)\n");
				exit(EXIT_FAILURE);
			}

			server_initialize(buffer, conn_s);
			netshell(buffer, conn_s);
		
			/* Close the connected socket by child process after echo job */
			if(close(conn_s) < 0){
	    			printf("AHS_SERVER_SHELL: Error calling close(conn_s)\n");
				exit(EXIT_FAILURE);
			}
			exit(EXIT_SUCCESS);
		}

		/*  Close the connected socket */

		if (close(conn_s) < 0) {
	    		printf("AHS_SERVER_SHELL: Error calling close(conn_s)\n");
	    		exit(EXIT_FAILURE);
		}
    	}
		
	return 0;
}

int client_shell(int argc, char **argv){
    	int conn_s;                
    	int port;                  
    	struct sockaddr_in servaddr;  
    	char buffer[MAX_LEN];      
    	char *endptr;
	struct hostent *he;
	struct in_addr address;
	int flag = 1;

	if(argc == 4){
		port = strtol(argv[3], &endptr, 0);
		if (*endptr) {
			printf("AHS_CLIENT_SHELL: Invalid port number.\n");
			exit(EXIT_FAILURE);
		}
		he = gethostbyname(argv[2]);
		if(he){
			bcopy(*he->h_addr_list, (char *)&address, sizeof(address));
		}
		else{
			herror("AHS_CLIENT_SHELL");
			exit(EXIT_FAILURE);
		}
	}
	else {
		printf("AHS_CLIENT_SHELL: Invalid arguments.\n");
		printf("Usage: ./ahs_shell --client Host Port\n");
		exit(EXIT_FAILURE);
    	}


    	if((conn_s = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
		printf("AHS_CLIENT_SHELL: Cannot creating listening socket.\n");
		exit(EXIT_FAILURE);
    	}

    	memset(&servaddr, 0, sizeof(servaddr));
    	servaddr.sin_family = AF_INET;
    	servaddr.sin_port = htons(port);
	servaddr.sin_addr = address;

    	if(connect(conn_s, (struct sockaddr *) &servaddr, sizeof(servaddr) ) < 0 ) {
		printf("AHS_CLIENT_SHELL: Cannot connect.\n");
		exit(EXIT_FAILURE);
    	}

	printf("Connected to %s:%d !\n", inet_ntoa(address), port);

    	//printf("Enter the string to echo: ");

	//writeline(conn_s, "hi", 1);

        while(flag) {
    		if(readline(conn_s, buffer, MAX_LEN-1) == 0){
			printf("AHS_CLIENT_SHELL: Connection closed by the other end.\n");
			break;
		}
		printf("%s",buffer);
    		fgets(buffer, MAX_LEN, stdin);
		if(!strcmp(buffer, "exit\n")){
			flag = 0;
		}
		strcat(buffer, END_OF_MESSAGE);
	    	writeline(conn_s, buffer, strlen(buffer));
		
		//writeline(conn_s, END_OF_MESSAGE, 1);
    	}

	return EXIT_SUCCESS;
}

void netshell(char *buffer, int conn){
	char *line;
	int flag = 1;
	COMMAND *command;
	server_help(buffer, conn);

	while(flag){
		line = (char *)malloc(MAX_LEN*sizeof(char));
		command = (COMMAND *)malloc(sizeof(int) + sizeof(char **) + sizeof(char *) + sizeof(boolean));
		writeline(conn, BASH_LINE, strlen(BASH_LINE));
		writeline(conn, END_OF_MESSAGE, 1);
		//printf("%s", BASH_LINE);
		readline(conn, line, MAX_LEN-1);
		//printf("the input line_len %d: \"%s\"", strlen(line), line);
		if(!strcmp(line, "\n")){
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
				sprintf(buffer, "%s\n", COMMAND_ERROR);
				writeline(conn, buffer, strlen(buffer));
			}
		}
		else if(!strcmp(command->com, HELP)){
			if(command->argc < 2){
				server_help(buffer, conn);
			}
			else{
				sprintf(buffer, "%s\n", COMMAND_ERROR);
				writeline(conn, buffer, strlen(buffer));
			}
		}
		/*else if(!strcmp(command->com, CLEAR)){
			if(command->argc < 2){
				system(CLEAR);
			}
			else{
				sprintf(buffer, "%s\n", COMMAND_ERROR);
				writeline(conn, buffer, strlen(buffer));
			}
		}*/
		else if(!strcmp(command->com, PWD)){
			if(command->argc > 1){
				sprintf(buffer, "%s\n", COMMAND_ERROR);
				writeline(conn, buffer, strlen(buffer));
			}
			else{
				sprintf(buffer, "%s\n", CWD);
				writeline(conn, buffer, strlen(buffer));
			}
		}
		else if(!strcmp(command->com, CD)){
			if(command->argc > 2){
				sprintf(buffer, "%s\n", COMMAND_ERROR);
				writeline(conn, buffer, strlen(buffer));
			}
			else if(command->argc == 1){
				server_change_directory(getenv("HOME"), buffer, conn);		
			}
			else if(!strcmp(command->argv[1], "..")){
				server_change_directory(get_parent_directory(CWD), buffer, conn);
			}
			else if(!strcmp(command->argv[1], ".")){
				server_change_directory(CWD, buffer, conn);
			}
			else{
				server_change_directory(command->argv[1], buffer, conn);
			}
		}
		else if(!strcmp(command->com, BGLIST)){
			if(command->argc > 1){
				sprintf(buffer, "%s\n", COMMAND_ERROR);	
				writeline(conn, buffer, strlen(buffer));
			}
			else{
				server_bgtask_list(buffer, conn);
			}
		}
		else if(!strcmp(command->com, BGSTART)){
			if(command->argc < 2){
				sprintf(buffer, "%s\n", COMMAND_ERRORC);	
				writeline(conn, buffer, strlen(buffer));
			}
			else{
				server_bgtask_kill(atoi(command->argv[1]), SIGCONT, buffer, conn);
			}
		}
		else if(!strcmp(command->com, BGSTOP)){
			if(command->argc < 2){
				sprintf(buffer, "%s\n", COMMAND_ERRORC);	
				writeline(conn, buffer, strlen(buffer));
			}
			else{
				server_bgtask_kill(atoi(command->argv[1]), SIGSTOP, buffer, conn);
			}
		}
		else if(!strcmp(command->com, BGKILL)){
			if(command->argc < 2){
				sprintf(buffer, "%s\n", COMMAND_ERRORC);	
				writeline(conn, buffer, strlen(buffer));
			}
			else{
				server_bgtask_kill(atoi(command->argv[1]), SIGTERM, buffer, conn);
			}
		}
		else{
			if(!strcmp(command->com, "bg") && command->argc < 2){
				sprintf(buffer, "%s\n", COMMAND_ERRORC);
				writeline(conn, buffer, strlen(buffer));
			}	
			else{
				server_execute(command, buffer, conn);
			}
		}
		server_check_bgtasks(buffer, conn);
		free(line);
		free(command);
	}
	deconstruct();
	return;
}

// execute the arbitrary command
int server_execute(COMMAND *command, char *buffer, int conn){
	pid_t pid = fork();
	
	if(pid == 0){	//child
		if(command->bg){
			//signal(SIGINT, server_handle_signal);
			//signal(SIGUSR1, server_handle_signal);
		}
		execvp(command->com, command->argv);
		sprintf(buffer, "%s: %s\n", command->com, COMMAND_ERROR);
		writeline(conn, buffer, strlen(buffer));
		exit(EXIT_FAILURE);
	}
	else if(pid > 0){ //parent
		int childStatus;
		if(!command->bg){
			IS_RUNNING_FGTASK = 1;
			waitpid(pid, &childStatus, WUNTRACED);
			waitpid(pid, &childStatus, WUNTRACED);
			//sprintf(buffer, "%s: %s\n");
			//writeline(conn, buffer, strlen(buffer));			
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
		sprintf(buffer, "CHILD_PROCESS creation failed!");		
		writeline(conn, buffer, strlen(buffer));
	}
	return 0;
}

//change directory
void server_change_directory(char *dir, char *buffer, int conn){
	int status = chdir(dir);
	if(status >= 0){
		set_bash_line();
	} else {
		sprintf(buffer, "%s %s\n", CD_ERROR, dir);
		writeline(conn, buffer, strlen(buffer));
	}
	return;
}

// check background tasks
void server_check_bgtasks(char *buffer, int conn){
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
			sprintf(buffer, " (%d) %s is terminated.\n", tmp->number, tmp->name);
			writeline(conn, buffer, strlen(buffer));
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
void server_bgtask_list(char *buffer, int conn){
	check_bgtasks();
	if(BGTASK_NUMBER == 0){
		sprintf(buffer, "There isn't any background task!\n");
		writeline(conn, buffer, strlen(buffer));
		return;
	}
	int i = 0;
	BGTASK *tmp = bgtask_head;
	for(; i < BGTASK_NUMBER; i++){
		char *state = (tmp->state == RUNNING)? "running.":"stopped.";
		sprintf(buffer, " (%d) %s is %s\n",tmp->number, tmp->name, state);
		writeline(conn, buffer, strlen(buffer));
		tmp = tmp->next;
	}
	sprintf(buffer, "\nTotal background jobs: %d\n", BGTASK_NUMBER);
	writeline(conn, buffer, strlen(buffer));
	return;
}

// bgtask kill with signals TERM, CONT, STOP 
void server_bgtask_kill(int number, int signal, char *buffer, int conn){
	BGTASK *tmp = get_bgtask_by_number(number);
	if(tmp == NULL){
		sprintf(buffer, "There is no process (%d)\n", number);
		writeline(conn, buffer, strlen(buffer));
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

//print the bash help
void server_help(char *buffer, int conn){
        sprintf(buffer, "\n.................. Welcome to AHS_SHELL ..................\n\n");
	writeline(conn, buffer, strlen(buffer));
        sprintf(buffer, "Runnig options:\n");
	writeline(conn, buffer, strlen(buffer));
        sprintf(buffer, "\t --server IP[defult-localhost] PORT[defult-2012]\n");
	writeline(conn, buffer, strlen(buffer));
        sprintf(buffer, "\t\tto run the server side of the shell.\n");
	writeline(conn, buffer, strlen(buffer));
        sprintf(buffer, "\t --client IP PORT\n");
	writeline(conn, buffer, strlen(buffer));
        sprintf(buffer, "\t\tto run the client side of the shell.\n");
	writeline(conn, buffer, strlen(buffer));
        sprintf(buffer, "\t without any option\n");
	writeline(conn, buffer, strlen(buffer));
        sprintf(buffer, "\t\tto run the shell locally.\n");
	writeline(conn, buffer, strlen(buffer));
        sprintf(buffer, "\nThe commands you can type inside the shell:\n");
	writeline(conn, buffer, strlen(buffer));
        sprintf(buffer, "\thelp -> to see this.\n");
	writeline(conn, buffer, strlen(buffer));
        sprintf(buffer, "\tclear-> to clear the screen.\n");
	writeline(conn, buffer, strlen(buffer));
        sprintf(buffer, "\texit -> to exit from the shell.\n");
	writeline(conn, buffer, strlen(buffer));
        sprintf(buffer, "\tand any valid command on linux!\n");
	writeline(conn, buffer, strlen(buffer));
        sprintf(buffer, "\n\t\t     --> Have fun! <--\n\n");
	writeline(conn, buffer, strlen(buffer));
        sprintf(buffer, "....................... AHS_SHELL .......................\n\n");
	writeline(conn, buffer, strlen(buffer));
	//writeline(conn, END_OF_MESSAGE, 1);
	return;
}

//print the input command in detail
void server_print_command(COMMAND *command, char *buffer, int conn){
	int argc = command->argc;
	sprintf(buffer, "command argc = %d\n", command->argc);
	writeline(conn, buffer, strlen(buffer));
	char **tmp = command->argv;
	while(argc--){
		printf("%s\n", *tmp++);
		writeline(conn, buffer, strlen(buffer));
	}
}

//print bgtask in detail
void server_print_bgtask(BGTASK *tmp, char *buffer, int conn){
	sprintf(buffer, "number = %d, pid = %d, name = %s\n", tmp->number, tmp->pid, tmp->name);
	writeline(conn, buffer, strlen(buffer));
	return;
}

//initial the bash
void server_initialize(char *buffer, int conn){
	//signal(SIGINT, SIG_IGN);
	BUFFER = buffer;
	CONN = conn;
	//signal(SIGINT, server_handle_signal);	
	CWD = (char *)malloc(100 * sizeof(char));
	BASH_LINE = (char *)malloc(100 * sizeof(char));
	gethostname(hostname, 20);
	//system(CLEAR);
	set_bash_line();
	return;
}

// handle ctrl+c signal
void server_handle_signal(int signo){

	signal(SIGINT, server_handle_signal);
	switch(signo){
	case (SIGINT):
		if(!IS_RUNNING_FGTASK){
			sprintf(BUFFER, "\n%s", BASH_LINE);
			writeline(CONN, BUFFER, strlen(BUFFER));
			fflush(stdout);
		}
		else {
			fflush(stdout);
			sprintf(BUFFER, "\n");
			writeline(CONN, BUFFER, strlen(BUFFER));
		}
		break;

	case (SIGUSR1):
		fflush(stdout);
		break;
	}
	return;
}

