#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int removeFromPath(char** paths, int pathSize, char* removePath){
    char** newPath = malloc(32 * sizeof(char*));
    int j = 0;
    for(int i = 0; i < pathSize; i++){
        if(strcmp(paths[i], removePath) != 0){
            newPath[j] = paths[i];
            j++;
        }
    }
    //no path was removed
    if(j == pathSize){
        return -1;
    }
    paths = newPath;
    return 0;
}

int runCommand(char *command[], int commandSize, char** paths, int pathSize)
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

    //loop throught the paths and use access to see if the command is in the path
    for(int i = pathSize-1; i >= 0; i--){
        char* testPath = malloc(128);
        strcpy(testPath, paths[i]);
        strcat(testPath, "/");
        strcat(testPath, command[0]);
        if(access(testPath, F_OK) != -1){
            execv(testPath, command);
        }
    }

    // if the command is not in the path, print error
    write(STDOUT_FILENO, "Error: command not found\n", 25);
    return -1;
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
    char** paths = malloc(32 * sizeof(char*));
    //add initial path
    paths[0] = malloc(5 * sizeof(char));
    strcpy(paths[0], "/bin");
    int pathSize = 1;   

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
                    if(commandSize < 2){
                        write(STDOUT_FILENO, "Error: unable to parse path command, provide add, remove or clear\n", 66);
                        continue;
                    }
                    //add path
                    if(strcmp(parsedCommand[1], "add") == 0){
                        if(commandSize != 3){
                            write(STDOUT_FILENO, "Error: unable to parse path command\n", 36);
                            continue;
                        }
                        paths[pathSize] = malloc(sizeof(parsedCommand[2]) * sizeof(char));
                        strcpy(paths[pathSize], parsedCommand[2]);
                        pathSize++;
                    }else if(strcmp(parsedCommand[1], "remove") == 0){ //remove path
                        if(commandSize != 3){
                            write(STDOUT_FILENO, "Error: unable to parse path command\n", 36);
                            continue;
                        }
                        
                        if(removeFromPath(paths, pathSize, parsedCommand[2]) == -1){
                            write(STDOUT_FILENO, "Error: path not found\n", 22);
                            continue;
                        }

                        pathSize--;

                    }else if(strcmp(parsedCommand[1], "clear") == 0){ //clear path
                        if(commandSize != 2){
                            write(STDOUT_FILENO, "Error: unable to parse path command\n", 36);
                            continue;
                        }
                        pathSize = 0;
                    }
                    else{
                        write(STDOUT_FILENO, "Error: only add, remove, or clear is supported\n", 36);
                    }
                    
                    continue;
                }

                int pid = fork();
                if(pid == 0){
                    //run command
                    runCommand(parsedCommand, commandSize, paths, pathSize);
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
