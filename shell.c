#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include <errno.h>

#define MAX_LINE 80 /* 80 chars per line, per command */


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

void get_data_arr(arg_data* ad, int arg_len, char** args, arg_data* data_arr, int sz) {
    ad->arg1 = args[0];
    ad->d_len = arg_len;

    for (int l = 0; l < arg_len; l++) {
        ad->arg_arr[l] = args[l];
    }
    data_arr[sz] = *ad;
}

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
        close(fd[1]);

        int res1 = execvp(pipe_ad->arg1, pipe_ad->arg_arr1);
        printf("%s", pipe_ad->arg1);
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
        close(fd[0]);
        
        int res2 = execvp(pipe_ad->arg2, pipe_ad->arg_arr2);
        printf("%s", pipe_ad->arg2);
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
    
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
    return 0;
}

int main(int argc, char *argv[])
{
    char *args[MAX_LINE/2 + 1];	/* command line has max of 40 arguments */
    int should_run = 1;		/* flag to help exit program*/
    char* style[2] = {"seq", "par"};
    int selected = 0;

    if (argc > 2) {
        printf("Too many arguments were provided\n");
        return 0;
    }

    int is_file = 0;
    char* last_cmd = (char*) malloc(MAX_LINE * sizeof(char));
    last_cmd = "!!";
    int has_allocated = 0;

    arg_data* ad1 = (arg_data*) malloc(sizeof(arg_data));
    arg_data* ad2 = (arg_data*) malloc(sizeof(arg_data));

    while (should_run) {

        char* cmd_arr[MAX_LINE/2 + 1];
        int cmd_len = 0;
        
        if (argc == 2) {
            is_file = 1;
            FILE* file = fopen(argv[1], "r");
            if (file == NULL) {
                printf("Could not find %s\n", argv[1]);
                exit(1);
            }
            char input[MAX_LINE * (MAX_LINE/2+1)]; /* max of 40 lines */
            
            get_finput(file, input);
            get_args(&cmd_len, cmd_arr, input, ";"); // Separates cmds by ;
        }

        if (!is_file) {
            char* input = (char*)malloc(MAX_LINE * sizeof(char*));
            get_input(style[selected], input); // Get input
            get_args(&cmd_len, cmd_arr, input, ";"); // Separates cmds by ;
        }

        arg_data* data_arr = (arg_data*) malloc(cmd_len * sizeof(arg_data));
        has_allocated = 1;
        int sz = 0;

        pthread_t th[sz];
        int th_c = 0;

        for (int i = 0; i < cmd_len; i++) {
            if (verify_blank(cmd_arr[i]))
                continue;
            
            char* args[MAX_LINE/2 + 1];
            int arg_len = 0;
            
            // Set last command
            if (strlen(cmd_arr[i]) >= 2) {
                if (strncmp(cmd_arr[i], "!!", 2) != 0) {
                    last_cmd = (char*) malloc(MAX_LINE * sizeof(char));
                    strcpy(last_cmd, cmd_arr[i]);
                }
                else if (strncmp(cmd_arr[i], "!!", 2) == 0) {
                    strcpy(cmd_arr[i], last_cmd);
                }
            }

            int is_composed = check_pipe(cmd_arr[i]);
            
            pipe_arg_data* pipe_ad = (pipe_arg_data*) malloc(sizeof(pipe_arg_data));

            if (is_composed && strlen(cmd_arr[i]) >= 3) {
                char* tok = strtok(cmd_arr[i], "|");

                while(tok != NULL) {
                    args[arg_len] = tok;
                    tok = strtok (NULL, "|");
                    arg_len = arg_len + 1;
                }

                memset(pipe_ad->arg_arr1, '\0', MAX_LINE); // Clears trash
                pipe_ad->d_len1 = 0;

                memset(pipe_ad->arg_arr2, '\0', MAX_LINE); // Clears trash
                pipe_ad->d_len2 = 0;

                get_pipe_data(pipe_ad, args); // Parses the text to pipe_data
            }

            if(!is_composed) {
                //Separates the raw text into string arrays
                get_args(&arg_len, args, cmd_arr[i], " ");

                //Creates new arg_data struct to be stored in data_arr
                arg_data* ad = (arg_data*) malloc(sizeof(arg_data));
                memset(ad->arg_arr, '\0', MAX_LINE); // Clears trash
                get_data_arr(ad, arg_len, args, data_arr, sz);
                sz = sz + 1;

                clear_args(arg_len, args);
            }
            
            if (!is_composed) {
                // Custom shell handler
                if (check_arg(data_arr[sz-1].arg1, "!!")){
                    printf("No commands\n");
                    continue;
                } else if (check_arg(data_arr[sz-1].arg1, "exit")) {
                    should_run = 0;
                    break;
                } else if (set_style(&data_arr[sz-1].d_len, data_arr[sz-1].arg_arr, &selected) != 0) {
                    continue;
                }
            }
            
            // Sequential approach
            if (!selected) {
                if (is_composed) {
                    int res = exec_pipe(pipe_ad);
                    if (res > 0) {
                        printf("An error occurred while executing pipe\n");
                    }
                } else {
                    exec_fork(&data_arr[sz-1]);
                }
            }
            
            // Parallel approach
            if (selected) {
                if (is_composed) {
                    if (pthread_create(&th[th_c], NULL, (void*) exec_pipe, &pipe_ad) != 0) {
                        fprintf(stderr, "Error pthread create %ld\n", th[th_c]);
                        exit(1);
                    } else {
                        th_c++;
                    }
                } else {
                    if (pthread_create(&th[th_c], NULL, (void*) exec_fork, &data_arr[sz-1]) != 0) {
                        fprintf(stderr, "Error pthread create %ld\n", th[th_c]);
                        exit(1);
                    } else {
                        th_c++;
                    }
                }
            }
        }

        for (int i = 0; i < th_c; i++) pthread_join(th[i], NULL);
        for (int i = 0; i < th_c; i++) th[i] = 0;
        
        memset(cmd_arr, '\0', cmd_len);
        free(data_arr);

        if (is_file) exit(0);
    }
	return 0;
}
