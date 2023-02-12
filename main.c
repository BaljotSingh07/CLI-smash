#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char* paths[] = {"/bin/", "/usr/bin/", NULL};

int runCommand(char* command){
    char* argToken;

    char* args[10];
   
    int i = 0;
    while( (argToken = strsep(&command," ")) != NULL ){
        if(argToken[0] == '\0')
            continue;

        args[i] = argToken;
        i++;
    }
    //make it a null terminated array
    args[i] = NULL;

    // check for build in command
    if(strcmp(args[0], "exit") == 0){
        exit(1);
    }
    
    if(strcmp(args[0], "cd") == 0){
        chdir(args[1]);
    }

    if(strcmp(args[0], "path") == 0){
        //add path

        //remove path

        //clear path
    }
   
   //run execv at user/bin
    char* path = malloc(128);
    strcpy(path, "/bin/");
    strcat(path, args[0]);
    execv(path, args);
    return 0;
    
}

int main(void)
{
    
    while(1){
        char* input = malloc(128); 

        printf("Smash> ");
        fgets(input, 128, stdin);

        //remove newline
        input[strlen(input) - 1] = '\0';

        char* commandToken;
        char* commands[10];

        int i = 0;
        while( (commandToken = strsep(&input,"&")) != NULL ){
            if(commandToken[0] == '\0')
                continue;
        
            commands[i] = commandToken;
            i++;
        }


        //run each commands parralelly
        for(int j = 0; j < i; j++){
            int pid = fork();
            if(pid == 0){
                runCommand(commands[j]);
                return 0;
            }
        }
        //wait for all children to finish
        for(int j = 0; j < i; j++){
            int status;
            wait(&status);
            
            //check for 256 flag which means quit
            if(status == 256){
                exit(0);
            }
        }  
        
    
    }
    return 0;
}
