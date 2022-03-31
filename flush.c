#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <stdbool.h>

char user_input;


void set_cwd(char* cwd)
{
    char cwd_array[200];
    if (getcwd(cwd_array, sizeof(cwd_array)) != NULL)
    {
        cwd = cwd_array;
    }
    else
    {
        printf("getcwd() error");
    }
}

char* prompt_user()
{
    scanf(" %c", &user_input);
    return &user_input;
}

int flush()
{
    char* cwd;
    char* input;
    set_cwd(cwd);

    while(1)
    {
        input = prompt_user();
        int pid = fork();
        
        // child
        if (pid == 0){
            printf("%s: \n", *cwd);
            // exec(3)...
        }

        // parent waiting for child
        else{
            waitpid(pid);
        }

        printf('%s \n', input);
    }
}