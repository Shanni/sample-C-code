/** @file shell.c */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "log.h"


log_t Log;


/**
 * Starting point for shell.
 */
int main() {
	
	ssize_t line_ct;
	size_t line_size = 0;
	char *line = NULL;
	char *temp = NULL;
	char *CEP = "Command executed by pid=%d\n";	


	log_init(&Log);	

	
	for (;;) {
		
		pid_t pid = getpid();

		char *CWD = getcwd(0,0);

		printf("(pid=%d)%s$ ", getpid(), CWD);
		fflush(stdout);
		
		line_ct = getline(&line, &line_size, stdin);
		
		line[--line_ct] = '\0';

		if (line_ct <= 0) continue;

		if ( !strcmp(line, "!#")) {			//Prints all commands stored in reverse
			int i;					//chronological order
			for (i = Log.end-1; i >= 0; i--) {
				printf("%s\n", Log.the_log[i]);
			}	  					
			printf(CEP, pid);
			continue;

	
		} else if (!strncmp(line, "!", 1)) {		//Searches log for command, executes command if	
								//exists
			temp = log_search(&Log, line+1);
			
			if (temp != NULL) {
				printf("%s matches %s\n", line+1, temp);
				temp = strdup(temp);
				line = temp;
 				//log_push(&Log, temp);
				//printf(CEP, pid);
			} else {
				printf("No Match\n");
				free(CWD);
				continue;
			}

		}
		 
		if (strcmp(line, "exit") == 0) {
			printf(CEP, pid);
			log_push(&Log, line);
			free(line);
			free(CWD);
			
			log_destroy(&Log);
			break;


		} else if (!strncmp(line, "cd ", 3)) {
			
			if (chdir(line + 3) == -1) {
				printf("%s: No such file or directory\n", line + 3);
			}
			printf(CEP, pid);
			log_push(&Log, line);
			free(CWD);
			continue;

		} else {
			log_push(&Log, line);
			pid_t child_pid = fork();
			char *command[500];
			char *t;
			
			int i;
			for (i = 0; i < 500; i++) {
				command[i] = NULL;
			}
			
			if (child_pid == 0) {

				pid_t cpid = getpid();
				printf(CEP, cpid);

				char delimiters[] = " !";	
				int j = 0;

				char *line_temp = line;

				while (t = strsep(&line_temp, delimiters)) {
					command[j] = t;
					j++;
				}


				execvp(command[0], command);
				printf("%s: not found\n", line);
				exit(1);

			} else {
				waitpid(child_pid, NULL, WUNTRACED);
				
			}
			
			
		}

		free(CWD);	
			

	}




	return 0;
}
