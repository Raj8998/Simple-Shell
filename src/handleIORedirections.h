#include "handlePipeCommands.h"
#include <fcntl.h>
#include <unistd.h>

int getIORedirectType(char **tokens);
void executeIOCommand(char **tokens);
int findFileNameIndex(char **tokens);

int getIORedirectType(char **tokens){
    int i=0;
    while(tokens[i] != NULL){
        if(strcmp(tokens[i], ">>") == 0){
            return 2;
        } else if(strcmp(tokens[i], ">") == 0){
            return 1;
        } else if(strcmp(tokens[i], "<") == 0){
            return 3;
        }
        ++i;
    }
    return 0;
}

int findFileNameIndex(char **tokens){
    int i=0;
    while(tokens[i] != NULL){
        if(strcmp(tokens[i], ">>") == 0 || strcmp(tokens[i], ">") == 0 || strcmp(tokens[i], "<") == 0){
            break;
        }
        ++i;
    }
    return i;
}

void executeIOCommand(char **tokens) {
    int redirectType = getIORedirectType(tokens);
    int filenameindex = findFileNameIndex(tokens);
    char *filename;
    if(tokens[filenameindex+1] != NULL) filename = tokens[filenameindex+1];
    else filename = NULL;
    tokens[filenameindex] = NULL;
    if(filename != NULL){
        pid_t pid = fork();

        if (pid == 0) { // Child process
            int fd;
            if (redirectType == 1) {  // Output redirection with ">"
                fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd < 0) {
                    perror("Could not open file!");
                    exit(1);
                }
                dup2(fd, STDOUT_FILENO);
                close(fd);
            } else if (redirectType == 2) {  // Output redirection with ">>"
                fd = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0644);
                if (fd < 0) {
                    perror("Could not open file!");
                    exit(1);
                }
                dup2(fd, STDOUT_FILENO);
                close(fd);
            } else if (redirectType == 3) {  // Input redirection with "<"
                fd = open(filename, O_RDONLY);
                if (fd < 0) {
                    perror("Could not open file!");
                    exit(1);
                }
                dup2(fd, STDIN_FILENO);
                close(fd);
            }

            execvp(tokens[0], tokens);
            perror("Error executing IO redirect!"); // If execvp fails
            exit(1);
        } else {
            waitpid(pid, NULL, WUNTRACED);
        }
    } else {
        perror("Invalid filename. No filename found!");
    }
}