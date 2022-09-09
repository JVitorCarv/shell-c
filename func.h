#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>

#define MAX_LINE 80

typedef struct {
    char* arg1;
    char* arg_arr[MAX_LINE];
    int d_len;
}arg_data;

typedef struct {
    char* arg1;
    char* arg_arr1[MAX_LINE];
    char* arg2;
    char* arg_arr2[MAX_LINE];
    int d_len1;
    int d_len2;
}pipe_arg_data;

void exec_fork(arg_data* data) {
    pid_t pid = fork();

    if (pid < 0) {
        printf("Fork failed\n");
        free(data);
    } else if (pid == 0) {
        int code = execvp(data->arg1, data->arg_arr);
        if (code < 0) {
            fprintf(stderr, "Error while trying to execute %s: %s\n", data->arg1, strerror(errno));
        }
        kill(getpid(), SIGKILL); /* Kills the process, so it doesn't keep existing */
    } else {
        wait(NULL);
    }
}

void get_args(int* arg_len, char** args, char* input, char* sep) {
    // Updates len, changes array
    char* tok = strtok(input, sep);

    while(tok != NULL) {
        args[*arg_len] = tok;
        tok = strtok (NULL, sep);
        *arg_len = *arg_len + 1;
    }
}

void clear_args(int arg_len, char** args) {
    for(int i = 0; i < arg_len; i++)
        args[i] = NULL;
}

// Refactor later
int set_style(int* arg_len, char** args, int* selected) {
    int len = strlen(args[0]);

    if (len >= 5) {
        if (strncmp("style", args[0], 5) == 0) {
            if (strcmp("sequential", args[1]) == 0) {
                *selected = 0;
                return 1;
            } else if (strcmp("s", args[1]) == 0) {
                *selected = 0;
                return 1;
            } else if (strcmp("seq", args[1]) == 0) {
                *selected = 0;
                return 1;
            } else if (strcmp("parallel", args[1]) == 0) {
                *selected = 1;
                return 1;
            } else if (strcmp("p", args[1]) == 0) {
                *selected = 1;
                return 1;
            } else if (strcmp("par", args[1]) == 0) {
                *selected = 1;
                return 1;
            } else {
                printf("%s is not a style.\n\nsequential\nparallel\n\nDid you type correctly?\n", args[1]);
                return -1;
            }
       }   
    }
    return 0;
}

/* Checks if the user wants to leave */
int check_arg(char* arg, char* comp) {
    if (strlen(arg) >= strlen(comp)) {
        if (strncmp(comp, arg, strlen(comp)) == 0) return 1;
    }
    return 0;
}

/* Checks if a given string contains only blank
characters */
int verify_blank(char* arg) {
    int b_count = 0;
    for(int i = 0; i < strlen(arg); i++) {
        if (isspace(arg[i]) != 0) b_count++;
    }
    if (b_count == strlen(arg)) return 1;
    return 0;
}

/* Checks if a given array contains a blank
item */
int has_blank(int arg_len, char** cmd_arr) {
    for(int i = 0; i < arg_len; i++) {
        if (verify_blank(cmd_arr[i])) return i;
    }
    return 0;
}

/* Verifies whether the given argument is a pipe */
int check_pipe(char* str) {
    for (int i = 0; i < strlen(str); i++) {
        if (str[i] == '|')  return 1;
    }
    return 0;
}

void rmv_n(char* str) {
    int len = strlen(str);
    if (isspace(str[len-1]) != 0) str[len-1] = 0;
}

/* get input from user keyboard */
void get_input(char* style, char* input) {
    printf("jvvc %s> ", style);
        fflush(stdout);
    if (fgets(input, MAX_LINE, stdin) == NULL) {
        printf("\n");
        exit(0);
    }
        fflush(stdin);
    rmv_n(input);
}

/* get input from file */
void get_finput(FILE* file, char* input) {
    int line_c = 0;
    char* line = (char*) malloc(MAX_LINE * sizeof(char*));

    while (fgets(line, MAX_LINE+2, file) && line_c < 40) {
        rmv_n(line);
        strcat(input, line);
        line_c = line_c + 1;
    }
    fclose(file);

    if (line_c >= 40) printf("Limit of 40 lines exceeded\n");    
}

/* Gets raw text and inserts it into an arg_data struct */
void get_data_arr(arg_data* ad, int arg_len, char** args, arg_data* data_arr, int sz) {
    ad->arg1 = args[0];
    ad->d_len = arg_len;

    for (int l = 0; l < arg_len; l++) {
        ad->arg_arr[l] = args[l];
    }
    data_arr[sz] = *ad;
}

/* Inserts raw text into a pipe arguments struct */
void get_pipe_data(pipe_arg_data* pipe_ad, char** args) {
    char* tok0 = strtok(args[0], " ");
    while(tok0 != NULL) {
        pipe_ad->arg_arr1[pipe_ad->d_len1] = tok0;
        pipe_ad->d_len1 = pipe_ad->d_len1 + 1;
        tok0 = strtok (NULL, " ");
    }
    pipe_ad->arg1 = pipe_ad->arg_arr1[0];

    char* tok1 = strtok(args[1], " ");
    while(tok1 != NULL) {
        pipe_ad->arg_arr2[pipe_ad->d_len2] = tok1;
        pipe_ad->d_len2 = pipe_ad->d_len2 + 1;
        tok1 = strtok (NULL, " ");
    }
    pipe_ad->arg2 = pipe_ad->arg_arr2[0]; 
}

/* Creates processes to pid1 and pid2 and executes commands */
int exec_pipe(pipe_arg_data* pipe_ad) {
    int fd[2];
    if (pipe(fd) < 0) {
        fprintf(stderr, "Pipe creation failed");
        return 1;
    }

    pid_t pid1 = fork();

    if (pid1 == 0) {
        close(STDOUT_FILENO);
        close(fd[0]);
        dup(fd[1]);

        int res1 = execvp(pipe_ad->arg1, pipe_ad->arg_arr1);
        if (res1 < 0) {
            fprintf(stderr, "Error while trying to execute %s: %s\n", pipe_ad->arg1, strerror(errno));
        }
    }

    if (pid1 < 0) {
        printf("Error while forking in pid1, exec_pipe()\n");
        return 1;
    }

    pid_t pid2 = fork();

    if (pid2 == 0) {
        close(STDIN_FILENO);
        close(fd[1]);
        dup(fd[0]);
        
        int res2 = execvp(pipe_ad->arg2, pipe_ad->arg_arr2);
        if (res2 < 0) {
            fprintf(stderr, "Error while trying to execute %s: %s\n", pipe_ad->arg2, strerror(errno));
        }
    }

    if (pid2 < 0) {
        printf("Error while forking in pid2, exec_pipe()\n");
        return 1;
    }

    close(fd[0]);
    close(fd[1]);
    
    /* Waits for both processes */
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
    return 0;
}