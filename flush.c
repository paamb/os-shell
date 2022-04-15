#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <stdbool.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


# define MAX_SIZE 300
# define MAX_STRING_LEN 10

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

void split_string(char *str, char *args[], char *cwd)
{
    int counter = 1;
    char delim[] = " \t";
    char *ptr = strtok(str, delim);
    args[0] = ptr;
    while (ptr != NULL)
    {  
        ptr = strtok(NULL, delim);
        if (ptr !=NULL){
            if (strcmp(ptr, ">") == 0){
                int fd;
                char absolute_path_to_file[MAX_SIZE];
                memset(absolute_path_to_file, '\0', sizeof(absolute_path_to_file));
                // printf("cwd: %s", cwd);
                strcat(absolute_path_to_file, cwd);
                ptr = strtok(NULL, delim);
                strcat(absolute_path_to_file, "/");
                strcat(absolute_path_to_file, ptr);
                // Tatt fra stackoverflow
                if(fd = open(absolute_path_to_file, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR)){
                    perror("Noe gikk galt kan ikke aapne fil");
                }
                close(0);
                dup2(fd, 1);
                break;
            }
            if (strcmp(ptr, "<") == 0){
                int fd0;
                char absolute_path_to_file[MAX_SIZE];
                memset(absolute_path_to_file, '\0', sizeof(absolute_path_to_file));
                strcat(absolute_path_to_file, cwd);
                ptr = strtok(NULL, delim);
                strcat(absolute_path_to_file,  "/");
                strcat(absolute_path_to_file, ptr);
                // Tatt fra stackoverflow
                if (fd0 = open(absolute_path_to_file, O_RDONLY) < 0){
                    perror("Cannot open file");
                    exit(0);
                }
                dup2(fd0, 0);
                close(0);
                return;
            }
        }
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

// void redirect_to_file()

int flush()
{
    char* cwd = malloc(MAX_SIZE);
    char* input = malloc(MAX_SIZE);
    char* input_pointer_array[3];
    while(1)
    {
        // set_cwd(cwd);
        // prompt_user(cwd, input);
        // char *args[4];
        // split_string(input, args);
        set_cwd(cwd);
        prompt_user(cwd, input);
        char *args[MAX_STRING_LEN];
        split_string(input, args, cwd);
        int pid = fork();
        
        
        // child
        if (pid == 0){
            // int counter = 1;
            // char delim[] = " \t";
            // char *ptr = strtok(input, delim);
            // args[0] = ptr;
            // while (ptr != NULL)
            // {
                
            //     ptr = strtok(NULL, delim);
            //     // ptr == ">"
            //     // ptr == "<"

            //     // Kjør feks ls > text.txt og ls vil bli sendt inn i text.txt
            //     if (strcmp(ptr, ">") == 0){
            //         int fd;
            //         printf("fdsafdsa\n");
            //         char full_path_to_file[MAX_SIZE];
            //         strcat(full_path_to_file, cwd);
            //         ptr = strtok(NULL, delim);
            //         strcat(full_path_to_file, "/");
            //         strcat(full_path_to_file, ptr);
            //         printf("%s\n", full_path_to_file);
            //         // Tatt fra stackoverflow
            //         fd = open(full_path_to_file, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
            //         close(1);
            //         dup2(fd, 1);
            //         // execvp(args[0], args);
            //         break;
            //     }
            //     if (strcmp(ptr, "<") == 0){
            //         int fd0;
            //         char full_path_to_file[MAX_SIZE];
            //         strcat(full_path_to_file, cwd);
            //         ptr = strtok(NULL, delim);
            //         strcat(full_path_to_file, "/");
            //         strcat(full_path_to_file, ptr);
            //         printf("%s fdsafdsa\n", full_path_to_file);
            //         // Tatt fra stackoverflow
            //         if (fd0 = open(full_path_to_file, O_RDONLY) < 0){
            //             perror("Cannot open file");
            //             exit(0);
            //         }
            //         printf("%d\n", fd0);
            //         close(0);
            //         dup2(fd0, 0);
            //         break;
            //     }
            //     if (strcmp(args[0], "cd") == 0)
            //     {
            //         chdir(args[1]);
            //     }

            //     args[counter] = ptr;
            //     counter++;

            // }
            // for (int i = 0; i < MAX_STRING_LEN; i++){
            //         printf("%s\n", args[i]);
            //     }
            // printf("haiehihfe\n");
            printf("hei printes\n");
            execvp(args[0], args);
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