#include <sys/wait.h>
#include <stdlib.h>
#include  <stdio.h>
#include <string.h>
#include <signal.h>
#include <setjmp.h>

// variable initializations
int MAX_BACKGROUND_PROCESSES=64;
static int currentForegroundProcess = -1;
static sigjmp_buf env;
static volatile sig_atomic_t jump_active = 0;

// Function definitions
int runCommand( char **tokens, int *background_processes);
int isCDCommand(const char *command);
int isExitCommand(const char *command);
int isBackgroundProcess( char **tokens);
void reapTerminatedBackgroundProcesses(int *background_processes);
int checkForBackgroundProcessAvailability(int *background_processes);
int findArrayLength(int *background_processes);
void deleteProcessFromList(int *background_processes, int i, int pid);
void addNewProcessToList(int *background_processes, int pid);
void killAllProcesses(int *background_processes);
void signal_handler(int sig);

// runCommand: Generalized function to execute shell commands
// @tokens: tokenized command input
// @background_processes: list of background processes
int runCommand( char **tokens, int *background_processes){
    // If no shell commands are passed then check for background process completion and reap if needed.
    if(tokens[0] == NULL){
        reapTerminatedBackgroundProcesses(background_processes);
        return 0;
    }

    // If "exit" command is given, then abort all processes
    if( isExitCommand(tokens[0]) ){
        killAllProcesses(background_processes);
        return 1;
    }

    // Logic for "cd" command: syntax must be `cd <folder-name>`
    if( isCDCommand(tokens[0]) ){
        if(tokens[2] != NULL){
            printf("Shell: Incorrect command\n");
        }
        chdir(tokens[1]);
        return 0;
    }

    // Check if command is to run as background, and add in background proceses if needed.
    int isBackgroundProcessEntered = isBackgroundProcess(tokens);
    if(isBackgroundProcessEntered && !checkForBackgroundProcessAvailability(background_processes)){
        printf("Shell: Reached the limit to have background processes. Please wait until 1 or more background processes are completed.\n");
        return 0;
    }

    // Create a new child process for running new command
    pid_t pid = fork();
	if(pid == 0){
        if(isBackgroundProcessEntered){
            setpgid(0, 0);
        }        

		execvp(tokens[0], tokens);
		printf("Shell: Incorrect command\n");
        exit(1);
	} else{
        if(!isBackgroundProcessEntered){
            currentForegroundProcess = pid;
            int status;
            waitpid(pid, &status,WNOHANG);
            while( ! WIFEXITED(status) ){
                waitpid(pid, &status, WNOHANG);
            }
        } else {
            printf("Shell: Process %u running in background.\n", pid);
            addNewProcessToList(background_processes, pid);
        }
        reapTerminatedBackgroundProcesses(background_processes);
	}
    return 0;
}

// reapTerminatedBackgroundProcesses: Function to terminate any terminated background 
// processes and free up the process queue
// @background_processes: list of background processes
void reapTerminatedBackgroundProcesses(int *background_processes){
    for(int i=0; i<MAX_BACKGROUND_PROCESSES; ++i){
        if( *(background_processes + i) == -1)
            continue;
        int status;
        int pid=*(background_processes + i);
        int rc_pid = waitpid(pid, &status, WNOHANG);
        if( rc_pid > 0 && WIFEXITED(status) ){
            printf("Shell: Background process with process id %u finished.\n", pid);
            deleteProcessFromList(background_processes, i, pid);
        }
    }
}

// # Helper Function
// checkForBackgroundProcessAvailability: Check if background process queue is available for new process or not
// @background_processes: list of background processes
int checkForBackgroundProcessAvailability(int *background_processes){
    if(findArrayLength(background_processes) < (int)MAX_BACKGROUND_PROCESSES)
        return 1;
    return 0;
}

// # Helper Function
// findArrayLength: Find the length of an array, given that the array is of static size, 
// and the elements can be in any available index
// @background_processes: list of background processes
int findArrayLength(int *background_processes){
    int length = 0;
    for(int i=0; i<MAX_BACKGROUND_PROCESSES; i++){
        if(*(background_processes + i) != -1)
            length++;
    }
    return length;
}

// # Helper Function
// deleteProcessFromList: Delete a background process from the queue given the process-id
// @background_processes: list of background processes
// @index: index to be deleted from
// @pid: process id of the process to be deleted
void deleteProcessFromList(int *background_processes, int index, int pid){
    if(*(background_processes + index) == pid){
        *(background_processes + index) = -1;
        return; 
    }
}

// # Helper Function
// addNewProcessToList: Add a new process in the background process queue given the process-id
// @background_processes: list of background processes
// @pid: process id of the process to be deleted
void  addNewProcessToList(int *background_processes, int pid){
    for(int i=0; i<MAX_BACKGROUND_PROCESSES; i++){
        if(*(background_processes+i) == -1){
            *(background_processes + i) = pid;
            return;
        }
    }
}

// # Helper Function
// killAllProcesses: Send SIGKILL singal to force kill all the processes
// @background_processes: list of background processes
void killAllProcesses(int *background_processes){
    for(int i=0; i<MAX_BACKGROUND_PROCESSES; ++i){
        if(*(background_processes+i) != -1){
            kill(*(background_processes+i), SIGKILL);
            waitpid(*(background_processes + i), NULL, 0);
            printf("Shell: Process %d killed during exit sequence.\n", *(background_processes+i));
            *(background_processes+i) = -1;
        }
    }
}

// # Helper Function
// isCDCommand: check if "cd" command is used
// @command: the command
int isCDCommand(const char *command){
    if(strcmp(command, "cd") == 0)
        return 1;
    return 0;
}

// # Helper Function
// isBackgroundProcess: check if command is for background-process
// @tokens: the command
int isBackgroundProcess( char **tokens){
    int i=0;
    while(tokens[i] != NULL){
        ++i;
    }
    i = i-1;
    if(strcmp(tokens[i], "&") == 0){
        tokens[i] =  NULL;
        return 1;
    }
    return 0;
}

// # Helper Function
// isExitCommand: check if command is "exit" command
// @command: the command
int isExitCommand(const char *command){
    if(strcmp(command, "exit") == 0)
        return 1;
    return 0;
}

// signal_handler: the Signal handler helper function to handle custom signals
// @sig: the integer signal code
void signal_handler(int sig){
    if(kill(currentForegroundProcess, SIGKILL) >= 0){
        waitpid(currentForegroundProcess, NULL, 0);
        printf("\nShell: Foreground process with PID:%u interrupted and now reaped by parent.\n",currentForegroundProcess);
    }
    signal(SIGINT, signal_handler);
    if (!jump_active) {
        return;
    }
    siglongjmp(env, 42);
}