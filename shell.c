#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>

#define MAX_LINE 80 /* 80 chars per line, per command */


typedef struct {
    char* arg1;
    char* arg_arr[MAX_LINE];
    int d_len;
}arg_data;

int exec_seq(char* command, char** arg_arr) {
    pid_t pid = fork();

    if (pid < 0) {
        printf("Fork failed\n");
        return 1;
    } else if (pid == 0) {
        int code = execvp(command, arg_arr);
        if (code == -1) {
            fprintf(stderr, "Error execvp()\n");
            return 1;
        }
    } else {
        wait(NULL);
    }
    return 0;
}

int exec_par(void* ad) {
    arg_data *data;
    data = (arg_data *) ad;

    pid_t pid = fork();

    if (pid < 0) {
        printf("Fork failed\n");
        return 1;
    } else if (pid == 0) {
        int code = execvp(data->arg1, data->arg_arr);
        if (code == -1) {
            fprintf(stderr, "Error execvp()\n");
            return 1;
        }
        free(ad);
    } else {
        wait(NULL);
    }

    return 0;
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
int verify_exit(char* arg) {
    if (strlen(arg) >= 4) {
        if (strncmp("exit", arg, 4) == 0) return 1;
    }
    return 0;
}

/* Checks if a given string contains only blank
characters */
int verify_blank(char* arg) {
    int b_count = 0;
    for(int i = 0; i < strlen(arg); i++) {
        if (isspace(arg[i]) != 0) {
            b_count++;
        }
    }
    if (b_count == strlen(arg)) {
        return 1;
    }
    return 0;
}

/* Checks if a given array contains a blank
item */
int has_blank(int arg_len, char** cmd_arr) {
    for(int i = 0; i < arg_len; i++) {
        if (verify_blank(cmd_arr[i])) {
            return i;
        }
    }
    return 0;
}

void rmv_n(char* str) {
    int len = strlen(str);
    if (isspace(str[len-1]) != 0) {
        str[len-1] = 0;
    }
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

    if (argc == 2) {
        FILE* file = fopen(argv[1], "r");
        if (file == NULL) {
            printf("Could not find %s\n", argv[1]);
            exit(1);
        }

        char* line_arr[MAX_LINE/2+1]; /* max of 40 lines */
        int line_c = 0;
        char input[MAX_LINE];

        while (fgets(input, MAX_LINE, file) && line_c < 40) {
            line_arr[line_c] = strdup(input);
            line_c = line_c + 1;
        }
        if (line_c >= 40) printf("Limit of 40 lines exceeded\n");
    
        for (int i = 0; i < line_c; i++) {
            rmv_n(line_arr[i]); /* remove \n */
            printf("%s\n", line_arr[i]);
        }

        printf("Closing %s\n", argv[1]);
        fclose(file);
        exit(0);
    }

    while (should_run) {
        printf("jvvc %s> ", style[selected]);
            fflush(stdout);

        char* input = malloc(MAX_LINE);      
        fgets(input, MAX_LINE, stdin);

        // Remove \n
        rmv_n(input);
        
        //Separates commands by ;
        char* cmd_arr[MAX_LINE/2 + 1];
        int cmd_len = 0;
        
        get_args(&cmd_len, cmd_arr, input, ";");

        int sz = 0;
        arg_data* data_arr = (arg_data*) malloc(cmd_len * sizeof(arg_data));

        for (int i = 0; i < cmd_len; i++) {
            if (verify_blank(cmd_arr[i])) {
                continue;
            }

            char* args[MAX_LINE/2 + 1];
            int arg_len = 0;

            get_args(&arg_len, args, cmd_arr[i], " ");

            arg_data* ad = (arg_data*) malloc(sizeof(arg_data));
            ad->arg1 = args[0];
            ad->d_len = arg_len;

            for (int l = 0; l < arg_len; l++) {
                ad->arg_arr[l] = args[l];
            }

            data_arr[sz] = *ad;
            sz++;
            clear_args(arg_len, args);
        }

        /* devbug print
        for (int i = 0; i < sz; i++) {
            printf("arg[%d] = %s\n", i, data_arr[i].arg1);
            for (int j = 0; j < data_arr[i].d_len; j++) {
                printf("%s ", data_arr[i].arg_arr[j]);
            }
            printf("\n");
        }*/

        // executes for every different command, sequential
        if (!selected) {
            for (int i = 0; i < sz; i++) {
                if (verify_exit(data_arr[i].arg1)) {
                    should_run = 0;
                    break; 
                } else if (set_style(&data_arr[i].d_len, data_arr[i].arg_arr, &selected) != 0) {
                    if (i > 0) {
                        printf("Style will be applied after all commands are finished...");
                    }
                    continue;
                }

                int res = exec_seq(data_arr[i].arg1, data_arr[i].arg_arr);
            }
        } else if (selected) {
            pthread_t th[sz];
            int break_flag = 0;
            for (int i = 0; i < sz; i++) {
                if (verify_exit(data_arr[i].arg1)) {
                    should_run = 0;
                    break_flag = 1;
                    break; 
                } else if (set_style(&data_arr[i].d_len, data_arr[i].arg_arr, &selected) != 0) {
                    if (i > 0) {
                        printf("Style will be applied after all commands are finished...");
                    }
                    break_flag = 1;
                    continue;
                } else {
                    if (pthread_create(&th[i], NULL, (void*) exec_par, (void*) &data_arr[i]) != 0) {
                        fprintf(stderr, "Error pthread create %ld\n", th[i]);
                        exit(1);
                    }
                }
            }
            if (!break_flag) {
                for (int i = 0; i < sz; i++) {
                    pthread_join(th[i], NULL);
                }
                for (int i = 0; i < sz; i++) {
                    th[i] = 0;
                }
            }
        }

        free(data_arr);
    }
	return 0;
}
