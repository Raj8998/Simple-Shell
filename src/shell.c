#include  <stdio.h>
#include  <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <setjmp.h>
#include <errno.h>
#include <limits.h>
#include "tokenizer.h"
#include "run_shell_commands.h"

#define MAX_INPUT_SIZE 1024


int *background_processes;

void initializeArray();
int handleCommandLineArguments(int argc, char* argv[]);

int main(int argc, char* argv[]) {
	// Handle commandline arguments
	if(handleCommandLineArguments(argc, argv) == 1){
		return 0;
	}

	char  line[MAX_INPUT_SIZE];            
	char  **tokens;              
	int i;

	// Initialize background process queue, for background tasks
	background_processes = (int *)malloc(MAX_BACKGROUND_PROCESSES*sizeof(int));
	initializeArray();

	/* Setup SIGINT */
	signal(SIGINT, signal_handler);

	while(1) {	
		
		if (sigsetjmp(env, 1) == 42) {
            printf("\n");
			continue;
        }
		jump_active = 1;

		/* BEGIN: TAKING INPUT */
		bzero(line, sizeof(line));
		printf("$ ");
		scanf("%[^\n]", line);
		getchar();

		/* END: TAKING INPUT */

		// Terminate with new line
		line[strlen(line)] = '\n';

		// Tokenize the input to run the command
		tokens = tokenize(line);

		// Run the command int background
		int status = runCommand(tokens, background_processes);
       
		// Freeing the allocated memory	
		for(i=0;tokens[i]!=NULL;i++){
			free(tokens[i]);
		}
		free(tokens);

		if(status == 1){
			free(background_processes);
			exit(0);
		}

	}
	return 0;
}

void initializeArray(){
	for(int i=0;i<MAX_BACKGROUND_PROCESSES;i++){
		background_processes[i] = -1;
	}
}

int handleCommandLineArguments(int argc, char* argv[]){
	if(argc > 1){
		if(strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0){
			char usage[256];  // Allocate enough space for the usage message
            snprintf(usage, sizeof(usage), "Usage: %s [OPTIONS]\n", argv[0]);  // Use snprintf for safe string formatting
            fprintf(stdout, "%s", usage);
            fprintf(stdout, "--help/-h: help\n");
            fprintf(stdout, "--max-background-processes [INT]: Set maximum background process queue size\n");
			return 1;
		} else if(strcmp(argv[1], "--max-background-processes") == 0){
			if(argc == 3){
				char *endptr;
                errno = 0; // To distinguish success/failure after call
                int val = strtol(argv[2], &endptr, 10);

                // Check for various possible errors
                if ((errno == ERANGE && (val == INT_MAX || val == INT_MIN))
                    || (errno != 0 && val == 0)) {
                    perror("strtol");
                    exit(EXIT_FAILURE);
                }

                if (endptr == argv[2]) {
                    fprintf(stderr, "Error: No digits were found\n");
                    exit(EXIT_FAILURE);
                }

                // If we got here, strtol() successfully parsed a number
                if (*endptr != '\0') {
                    // Further characters after number: invalid input
                    fprintf(stderr, "Error: Invalid integer value '%s'\n", argv[2]);
                    exit(EXIT_FAILURE);
                }

                printf("Max background processes set to: %d\n", val);
				MAX_BACKGROUND_PROCESSES=val;
				return 0;
            } else {
                fprintf(stderr, "Error: --max-background-processes requires an integer value\n");
				exit(EXIT_FAILURE);
            }
		} else{
			fprintf(stdout, "Invalid argument passed!\n\n");
			char usage[256];  // Allocate enough space for the usage message
            snprintf(usage, sizeof(usage), "Usage: %s [OPTIONS]\n", argv[0]);  // Use snprintf for safe string formatting
            fprintf(stdout, "%s", usage);
            fprintf(stdout, "--help/-h: help\n");
            fprintf(stdout, "--max-background-processes [INT]: Set maximum background process queue size\n");
			return 1;
		}
	}
	return 0;
}