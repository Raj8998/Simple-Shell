#include <sys/wait.h>
#include <stdlib.h>
#include  <stdio.h>
#include <string.h>
#include <signal.h>
#include <setjmp.h>
void closeAllPipes(int pipes[][2], int n);
char ***splitCommands(char **tokens);
void executePipedCommands(char ***commands);
int findPipe(char **tokens, int start_index);
int sizeofCharArray(char **array);

char ***splitCommands(char **tokens) {

    size_t bufsize = 1;
    int token_len = sizeofCharArray(tokens);
    char ***result = (char ***) malloc(sizeof(char **) * bufsize);

    int command_id = 0;
    int curr_index = 0;
    int next_pipe_index = 0;
    while (next_pipe_index >= 0) {
        if ((size_t) command_id >= bufsize) {
            result = (char ***) realloc(result, sizeof(char **) * ++bufsize);
        }
        next_pipe_index = findPipe(tokens, curr_index);
        char **command = NULL;
        if (next_pipe_index != -1) {
            command = (char **) malloc(sizeof(char *) * (next_pipe_index - curr_index));
            tokens[next_pipe_index] = NULL;
        } else {
            command = (char **) malloc(sizeof(char *) * (token_len - curr_index + 1));
        }
        command = tokens + curr_index;
        result[command_id++] = command;
        curr_index = next_pipe_index + 1;
    }
        
    return result;
}

void closeAllPipes(int pipes[][2], int n) {
    int i = 0;
    for (i = 0; i < n; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }
}

int sizeofCharArray(char **array) {

    int size = 0;
    for (size = 0; array[size] != NULL; size++);
    return size;
}

int findPipe(char **tokens, int start_index) {

    int i = start_index;
    while (tokens[i] != NULL) {
        if (strcmp(tokens[i], "|") == 0) {
            return i;
        }
        i++;
    }

    return -1;
}

void executePipedCommands(char ***commands) {
    
    int totalCommands = 0;
    for (totalCommands = 0; commands[totalCommands] != NULL; totalCommands++);

    int pipes[totalCommands][2];

    int pipe_index = 0;
    for (pipe_index = 0; pipe_index < totalCommands; pipe_index++) {
        pipe(pipes[pipe_index]);
    }

    pid_t pids[totalCommands];
    int index = 0;
    for (index = 0; index < totalCommands; index++) {
        pid_t pid;

        if ((pid = fork()) < 0) {
            printf("ERROR: forking child process failed\n");
            exit(1);
        }

        if (pid == 0) {
            if (index > 0) {
                dup2(pipes[index-1][0], STDIN_FILENO);
            }
            if (index + 1 < totalCommands) {
                dup2(pipes[index][1], STDOUT_FILENO);
            }

            closeAllPipes(pipes, totalCommands);

            if (execvp(*commands[index], commands[index])) {
                printf("ERROR: exec child process failed\n");
                exit(1);
            }
        } else {
            pids[index] = pid;
        }
    }

    closeAllPipes(pipes, totalCommands);

    int i = 0;
    for (i = 0; i < totalCommands; i++) {
        waitpid(pids[i], NULL, WUNTRACED);
    }
}