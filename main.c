#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

char *paths[] = {"/bin/", "/usr/bin/", NULL};

int runCommand(char *command[], int commandSize)
{
    // see if the redirection symbol is in the command and where is it
    int indexOfRedirection = -1;
    for(int i = 0; i < commandSize; i++){
        if(strcmp(command[i], ">") == 0){
            indexOfRedirection = i;
            break;
        }
    }

    if(indexOfRedirection != -1){
        
        // check for redirection formating
        if(indexOfRedirection != commandSize - 2){
            write(STDOUT_FILENO, "Error: unable to parse redirection command\n", 42);
            return -1;
        }
        if(indexOfRedirection == 0){
            write(STDOUT_FILENO, "Error: no command specified\n", 27);
            return -1;
        }
        // formating is good, redirect output

        int tmpFd = open(command[indexOfRedirection + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);

        dup2(tmpFd, STDOUT_FILENO);
        dup2(tmpFd, STDERR_FILENO);

        // remove redirection symbols from command
        command[indexOfRedirection] = NULL;
        close(tmpFd);
    }

   
    char *path = malloc(128);
    strcpy(path, "/bin/");
    strcat(path, command[0]);
    execv(path, command);
}

int getSeqCommands(char* buffer, char* seqCommands[]){
    int i = 0;
    char* token = strtok(buffer, ";");
    while(token != NULL){
        seqCommands[i] = token;
        i++;
        token = strtok(NULL, ";");
    }
    seqCommands[i] = NULL;
    return i;
}

int getAsyncCommands(char* buffer, char* asyncCommands[]){
    int i = 0;
    char* token = strtok(buffer, "&");
    while(token != NULL){
        asyncCommands[i] = token;
        i++;
        token = strtok(NULL, "&");
    }
    asyncCommands[i] = NULL;
    return i;
}

int parseCommand(char* unparsedCommand, char* parsedCommand[]){
    int i = 0;
    char* token = strtok(unparsedCommand, " ");
    while(token != NULL){
        parsedCommand[i] = token;
        i++;
        token = strtok(NULL, " ");
    }
    parsedCommand[i] = NULL;
    return i;
}


int main(void)
{
    
    while(1){
        char* buffer = malloc(128); 

        printf("Smash> ");
        fgets(buffer, 128, stdin);

        //remove newline
        buffer[strlen(buffer)-1] = '\0';

        char* seqCommands[10];
        int seqCommandSize = getSeqCommands(buffer, seqCommands);

        for(int i = 0; i < seqCommandSize; i++){
            char* asyncCommands[10];
            int asyncCommandSize = getAsyncCommands(seqCommands[i], asyncCommands);
            for(int j = 0; j < asyncCommandSize; j++){
                //check for built in commands
                
                //parse command
                char* parsedCommand[10];
                int commandSize = parseCommand(asyncCommands[j], parsedCommand);

                if(strcmp(parsedCommand[0], "exit") == 0){
                    exit(0);
                }
                else if(strcmp(parsedCommand[0], "cd") == 0){
                    chdir(parsedCommand[1]);
                    continue;
                }
                else if(strcmp(parsedCommand[0], "path") == 0){
                    
                    continue;
                }

                int pid = fork();
                if(pid == 0){
                    //run command
                    runCommand(parsedCommand, commandSize);
                    exit(0);
                }
            }
            for(int j = 0; j < asyncCommandSize; j++){
                wait(NULL);
            }
        }
    
    }
    return 0;
}
