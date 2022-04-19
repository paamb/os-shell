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

#define MAX_SIZE 300
#define MAX_ARGS 10

struct Background_p
{
    pid_t pid;
    struct Background_p *next_process;
    char *command;
};

void push_process(struct Background_p **head_process, pid_t pid, char *command)
{
    char *command_copy = malloc(MAX_SIZE);
    strcpy(command_copy, command);

    // Mallocing the process
    struct Background_p *process = (struct Background_p *)malloc(sizeof(struct Background_p));

    process->pid = pid;

    process->command = command_copy;

    // Attaching head to incoming process
    process->next_process = (*head_process);
    // Head is now incoming process
    (*head_process) = process;
}

// Printing the status when exiting
void print_status(int status, char *command)
{
    if (WIFEXITED(status))
    {
        int exit_status = WEXITSTATUS(status);
        printf("Exit status [%s] = %d \n", command, exit_status);
    }
}

// Waits for background processes with WNOHANG. Removes from linked list if process is finished.
void wait_for_background_processes(struct Background_p **head_process)
{
    int status;
    struct Background_p *curr_process = *head_process;
    struct Background_p *prev_process;

    while (curr_process != NULL && curr_process->pid > 0)
    {
        if (waitpid(curr_process->pid, &status, WNOHANG) != 0)
        {
            if (curr_process == *head_process)
            {
                *head_process = curr_process->next_process;
                print_status(status, curr_process->command);
                free(curr_process);
                curr_process = *head_process;
            }
            else
            {
                prev_process->next_process = curr_process->next_process;
                print_status(status, curr_process->command);
                free(curr_process);
            }
        }
        else
        {
            prev_process = curr_process;
            curr_process = curr_process->next_process;
        }
    }
}

void set_cwd(char *cwd)
{
    char cwd_array[200];

    if (getcwd(cwd_array, sizeof(cwd_array)) != NULL)
    {
        strcpy(cwd, cwd_array);
    }
    else
    {
        printf("getcwd() error");
    }
}

void prompt_user(char *cwd, char *input)
{
    printf("%s: ", cwd);

    if (fgets(input, MAX_SIZE, stdin) == NULL)
    {
        printf("\n");
        exit(0);
    }
    if (input[0] != '\n')
    {
        input[strcspn(input, "\n")] = 0;
    }
}

// Enables redirection for child process
void redirection(char *args[], char *cwd)
{
    int fd;
    int i = 0;
    int redirect = 0;
    int command_ends = 0;
    char absolute_path_to_file[MAX_SIZE];

    while (args[i] != NULL)
    {
        memset(absolute_path_to_file, '\0', sizeof(absolute_path_to_file));
        if (strcmp(args[i], ">") == 0 || strcmp(args[i], "<") == 0)
        {
            strcat(absolute_path_to_file, cwd);
            strcat(absolute_path_to_file, "/");
            strcat(absolute_path_to_file, args[i + 1]);
            redirect = 1;
            if (command_ends == 0)
            {
                command_ends = i;
            }
            if (strcmp(args[i], ">") == 0)
            {
                fd = open(absolute_path_to_file, O_TRUNC | O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
                if (fd < 0)
                {
                    perror("Something went wrong, cannot open file");
                }
                dup2(fd, 1);
                close(fd);
            }
            if (strcmp(args[i], "<") == 0)
            {
                fd = open(absolute_path_to_file, O_RDONLY);
                if (fd < 0)
                {
                    perror("Cannot open file");
                    exit(0);
                }
                dup2(fd, 0);
                close(fd);
            }
        }
        i++;
    }
    if (redirect)
    {
        for (int j = command_ends; j < MAX_ARGS; j++)
        {
            args[j] = '\0';
        }
    }
}

// Checks if process is background process
int is_background_process(char *args[])
{
    for (int i = 0; i < MAX_ARGS; i++)
    {
        if (args[i] == NULL)
        {
            return 0;
        }
        if (strcmp(args[i], "&") == 0 && args[i + 1] == NULL)
        {
            args[i] = '\0';
            return 1;
        }
    }
    return 0;
}

// Called when 'jobs' is put in command line. Prints background processes
void print_processes(struct Background_p *process)
{
    printf("\nRunning processes\n");
    printf("-----------------\n");

    while (process != NULL)
    {
        printf("PID: %d, Command: [", process->pid);
        printf("%s", process->command);
        printf("]\n");
        process = process->next_process;
    }
    printf("\n\n");
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
}

int flush()
{
    char *cwd = malloc(MAX_SIZE);
    char *input = malloc(MAX_SIZE);
    char *command = malloc(MAX_SIZE);
    char *input_pointer_array[3];
    int background_process = 0;
    int status;

    // Initializing linked list
    struct Background_p *head = NULL;

    while (1)
    {
        background_process = 0;
        set_cwd(cwd);
        wait_for_background_processes(&head);
        prompt_user(cwd, input);
        strcpy(command, input);
        char *args[MAX_ARGS];
        split_string(input, args, cwd);
        background_process = is_background_process(args);

        int pid = fork();

        // child
        if (pid == 0)
        {
            if (strcmp(args[0], "jobs") == 0)
            {
                print_processes(head);
            }
            redirection(args, cwd);
            execvp(args[0], args);
            exit(0);
        }

        else if (pid < 0)
        {
            printf("Error while creating child");
        }

        else
        {
            if (!background_process)
            {
                waitpid(pid, &status, 0);
                print_status(status, command);
            }
            else
            {
                push_process(&head, pid, command);
            }
        }
    }
}