/*****************************************************
*                                                    *
*            Simple shell written in C.              *
*       by Amir Hossein Sojoodi, (C) 2012            *
*                                                    *
*                                                    *
******************************************************/

#include "ahs_parser.h"

void parse(char *line, COMMAND *command){
        int counter = 0, index = 0;
        char *token;

        command->com = (char *)malloc(100 * sizeof(char));
        command->argv = (char **)malloc(100 * sizeof(char*));

        while((index = ahs_strtok(line, " \n\r", &token, index)) != -1){
                command->argv[counter] = (char *)malloc(strlen(token)*sizeof(char));
                strcpy(command->argv[counter++], token);
                //printf("token %d = %s\n", counter, token);//for verify the parsing
        }
	if(!strcmp(command->argv[0], "bg") && counter > 1){
        	strcpy(command->com, command->argv[1]);
		command->argv++;
		command->bg = true;
		command->argc = counter - 1;
	}
	else{
		strcpy(command->com, command->argv[0]);
		command->bg = false;
        	command->argc = counter;
	}
	//strcpy(command->argv[counter], (char *) 0);
        return;
}

int ahs_strtok(char *src, char *delim, char **token, int start){
        int count = start;
        int len = 0, i, j;
        while(src[count] != '\0' && src[count] == delim[0]){
                count++;
        }
        while(src[count] != '\0' && src[count] != delim[0] && src[count] != delim[1] && src[count] != delim[2]) {
                len++;
                count++;
        }
        (*token) = (char *)malloc(len * sizeof(char) + 1);
        for(i = 0, j = count - len; i < len; i++, j++){
                (*token)[i] = src[j];
        }
        (*token)[i] = '\0';
        if(count == start || len == 0)
                return -1;
        return count;
}

char *get_parent_directory(char *CWD){
	int i;
	for(i = strlen(CWD) - 1; i > 0 && CWD[i] != '/' ; i--)
		;
	CWD[i + 1] = '\0';
	return CWD;
}

ssize_t readline(int sockd, void *vptr, size_t maxlen) {
    	ssize_t n, rc;
    	char    c, *buffer;

    	buffer = vptr;

    	for ( n = 1; n < maxlen; n++ ) {
	
		if ( (rc = read(sockd, &c, 1)) == 1 ) {
	    		if (/*c == '\n' ||*/ c == '$')
				break;
			*buffer++ = c;
		}
		else if ( rc == 0 ) {
	    		if ( n == 1 )
				return 0;
			    else
				break;
		}
		else {
			if ( errno == EINTR )
				continue;
	    		return -1;
		}
    	}

    	*buffer = 0;
    	return n;
}


/*  Write a line to a socket  */

ssize_t writeline(int sockd, const void *vptr, size_t n) {
    	size_t      nleft;
    	ssize_t     nwritten;
    	const char *buffer;

    	buffer = vptr;
    	nleft  = n;

    	while ( nleft > 0 ) {
		if ( (nwritten = write(sockd, buffer, nleft)) <= 0 ) {
	    		if ( errno == EINTR )
				nwritten = 0;
	    		else
				return -1;
		}
		nleft  -= nwritten;
		buffer += nwritten;
    	}
	//write(sockd, "$", 1);
    	return n;
}

void getline(char *line, int length) {
    	char *linep = line;
    	size_t lenmax = length, len = lenmax;
    	int c;

    	for(;;) {
        	c = fgetc(stdin);
        	if(c == EOF)
            		break;

        	if(--len == 0) {
            		char * linen = realloc(linep, lenmax *= 2);
            		len = lenmax;

            		if(linen == NULL) {
                		free(linep);
                		return;
            		}
            		line = linen + (line - linep);
        		linep = linen;
        	}

        	if((*line++ = c) == '\n')
        	break;
    	}
    	*line = '\0';
	return;
}





