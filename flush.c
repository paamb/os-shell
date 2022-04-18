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

struct Background_p {
    int pid;
    struct Background_p* next;    
};
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

void redirection(char *args[], char *cwd){
    int fd;
    int i = 0;
    int redirect = 0;
    int command_ends = 0;
    char absolute_path_to_file[MAX_SIZE];
    
    while (args[i] != NULL){
        memset(absolute_path_to_file, '\0', sizeof(absolute_path_to_file));
        if (strcmp(args[i],">") == 0 || strcmp(args[i],"<") == 0){
            strcat(absolute_path_to_file, cwd);
            strcat(absolute_path_to_file, "/");
            strcat(absolute_path_to_file, args[i+1]);
            redirect = 1;
            if (command_ends == 0){
                command_ends = i;
            } 
            if (strcmp(args[i], ">") == 0){
                fd = open(absolute_path_to_file, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
                if(fd < 0){
                    perror("Noe gikk galt kan ikke aapne fil");
                }
                printf("%d\n", fd);

                // fd er naa output
                dup2(fd, 1);
                close(fd);
            }
            if (strcmp(args[i], "<") == 0){
                // Tatt fra stackoverflow
                fd = open(absolute_path_to_file, O_RDONLY);
                if (fd < 0){
                    perror("Cannot open file");
                    exit(0);
                }
                dup2(fd, 0);
                close(fd);
            }
        }
        i++;
    }
    //Slicer slik at man faar inn riktig argumenter i execvp
    if (redirect){
        for (int j = command_ends; j < MAX_STRING_LEN; j++){
            args[j] = '\0';
        }
    }
}

int is_background_process(char *args[]){
    for (int i = 0; i < MAX_STRING_LEN; i++){
        if (strcmp(args[i], "&") == 0 && args[i+1] == NULL){
            args[i] = '\0';
            return 1;
        } 
    }
    return 0;
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
        
        args[counter] = ptr;
        counter++;
    }
    if (strcmp(args[0], "cd") == 0)
    {
        chdir(args[1]);
    };
    
    // for (int i = 0; i < MAX_STRING_LEN; i++){
    //     printf("Lines: %s\n", args[i]);
    //     if (args[i] == 0){
    //         break;
    //     }
    //     // if (strcmp(args[i], "&") == 0 && strcmp(args[i+1]) == '\0') 
    // }
    // Maa ende paa null men ser ut til at det skjer uansett
    // args[counter] = NULL;
}


int flush()
{
    char* cwd = malloc(MAX_SIZE);
    char* input = malloc(MAX_SIZE);
    char* input_pointer_array[3];
    int background_process = 0;

    // Initializing linked list
    struct Background_p* head = NULL;
    
    while(1)
    {
        background_process = 0;
        set_cwd(cwd);
        prompt_user(cwd, input);
        char *args[MAX_STRING_LEN];
        split_string(input, args, cwd);
        background_process = is_background_process(args);
        printf("%d\n", background_process);
        int pid = fork();
        
        
        // child
        if (pid == 0){
            if (background_process){
                
            }
            redirection(args, cwd);
            execvp(args[0], args);
            // Man kommer bare hit om execvp failer
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