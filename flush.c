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
#define MAX_STRING_LEN 10

struct Background_p
{
    pid_t pid;
    struct Background_p *next_process;
    char *arguments;
};

void push_process(struct Background_p **head_process, pid_t pid, char *command)
{
    char *arguments = malloc(MAX_SIZE);
    strcpy(arguments, command);
    // Bygger opp foran og går bakover
    struct Background_p *process = (struct Background_p *)malloc(sizeof(struct Background_p));

    process->pid = pid;

    process->arguments = arguments;
    // Peker på NULL ved start
    process->next_process = (*head_process);
    // Head blir nå
    (*head_process) = process;
}

void print_status(int status, char *command)
{
    if (WIFEXITED(status))
    {
        int exit_status = WEXITSTATUS(status);
        printf("Exit status [%s] = %d \n", command, exit_status);
    }
}

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
                print_status(status, curr_process->arguments);
                free(curr_process);
                curr_process = *head_process;
            }
            else
            {
                prev_process->next_process = curr_process->next_process;
                print_status(status, curr_process->arguments);
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
    // bzero((char *)input, MAX_SIZE);
    printf("%s: ", cwd);

    if (fgets(input, MAX_SIZE, stdin) == NULL)
    {
        printf("\n");
        exit(0);
    }
    // Remove newline character from input

    if (input[0] != '\n')
    {
        input[strcspn(input, "\n")] = 0;
    }
}

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
                // | S_IWUSR
                // O_RDWR | O_CREAT, S_IRUSR
                fd = open(absolute_path_to_file, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR);
                if (fd < 0)
                {
                    perror("Noe gikk galt kan ikke aapne fil");
                }
                // fd er naa output
                dup2(fd, 1);
                close(fd);
            }
            if (strcmp(args[i], "<") == 0)
            {
                // Tatt fra stackoverflow
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
    // Slicer slik at man faar inn riktig argumenter i execvp
    if (redirect)
    {
        for (int j = command_ends; j < MAX_STRING_LEN; j++)
        {
            args[j] = '\0';
        }
    }
}

int is_background_process(char *args[])
{
    for (int i = 0; i < MAX_STRING_LEN; i++)
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

void print_processes(struct Background_p *process)
{
    printf("\nRunning processes\n");
    printf("-----------------\n");

    while (process != NULL)
    {
        printf("PID: %d, Command: [", process->pid);
        printf("%s", process->arguments);
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
        char *args[MAX_STRING_LEN];
        split_string(input, args, cwd);
        background_process = is_background_process(args);
        // printf("I flush: %s\n", input);

        int pid = fork();

        // child
        if (pid == 0)
        {
            // dup2(p[1], 1);
            if (strcmp(args[0], "jobs") == 0)
            {
                print_processes(head);
            }
            redirection(args, cwd);
            execvp(args[0], args);
            // Man kommer bare hit om execvp failer
            exit(0);
        }

        // parent waiting for child
        else if (pid < 0)
        {
            printf("Error while creating child");
        }
        {
            // TODO: Fix head
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