#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <stdbool.h>
# define MAX_SIZE 300

void set_cwd(char* cwd)
{
    char cwd_array[200];

    if (getcwd(cwd_array, sizeof(cwd_array)) != NULL)
    {
        strcpy(cwd, cwd_array);
        // printf("%s \n", cwd);
    }
    else
    {
        printf("getcwd() error");
    }
}

void prompt_user(char * cwd, char * input)
{
    // bzero((char *)input, MAX_SIZE);
    printf("%s: ", cwd);
    
    if (fgets(input, MAX_SIZE, stdin) == NULL){
        printf("\n");
        exit(0);
    }
    //Remove newline character from input

    if (input[0] != '\n'){
        input[strcspn(input, "\n")] = 0;
    }
}

void split_string(char *str, char *args[])
{
    int counter = 1;
    char delim[] = " \t";
    char *ptr = strtok(str, delim);
    // printf("%s \n", ptr);
    args[0] = ptr;
    while (ptr != NULL)
    {
        ptr = strtok(NULL, delim);
        // ptr == ">"
        // ptr == "<"
        args[counter] = ptr;
        counter++;
    }
    if (strcmp(args[0], "cd") == 0)
    {
        chdir(args[1]);
    };
    // Maa ende paa null men ser ut til at det skjer uansett
    // args[counter] = NULL;
}

int flush()
{
    char* cwd = malloc(MAX_SIZE);
    char* input = malloc(MAX_SIZE);
    char* input_pointer_array[3];
    while(1)
    {
        set_cwd(cwd);
        prompt_user(cwd, input);
        char *args[4];
        split_string(input, args);
        int pid = fork();
        
        
        // child
        if (pid == 0){
            printf("Child: %d \n", pid);
            execv(args[0], args);
            exit(0);

        }

        // parent waiting for child
        else{
            int status;
            waitpid(pid, &status, 0);
            printf("Parent: %d \n", pid);
            if(WIFEXITED(status)){
                int exit_status = WEXITSTATUS(status);
                printf("Exit status = %d \n", exit_status);
            }
        }
    }
}