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

int exec_fork(void* ad) {
    arg_data *data;
    data = (arg_data *) ad;

    pid_t pid = fork();

    if (pid < 0) {
        printf("Fork failed\n");
        free(ad);
        return 1;
    } else if (pid == 0) {
        int code = execvp(data->arg1, data->arg_arr);
        if (code == -1) {
            fprintf(stderr, "Error while trying to execute %s: %s\n", data->arg1, strerror(errno));
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

void rmv_n(char* str) {
    int len = strlen(str);
    if (isspace(str[len-1]) != 0) str[len-1] = 0;
}

/* get input from user keyboard */
void get_input(char* style, char* input) {
    printf("jvvc %s> ", style);
        fflush(stdout);
    fgets(input, MAX_LINE, stdin);
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
    char* last_command = "No commands";

    while (should_run) {

        //Separates commands by ;
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
            get_args(&cmd_len, cmd_arr, input, ";");
        }

        if (!is_file) {
            char* input = (char*)malloc(MAX_LINE * sizeof(char*));
            get_input(style[selected], input); // Get input
            get_args(&cmd_len, cmd_arr, input, ";");
        }

        arg_data* data_arr = (arg_data*) malloc(cmd_len * sizeof(arg_data));
        int sz = 0;

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
            // Does not add to last_command if last command asked was history
            if (strlen(data_arr[sz].arg1) >= 2){
                if (strncmp(data_arr[sz].arg1, "!!", 2) != 0) {
                    last_command = data_arr[sz].arg1;
                }
            }
            sz++;
            clear_args(arg_len, args);
        }
        clear_args(cmd_len, cmd_arr);

        pthread_t th[sz];
        int th_c = 0;

        for (int i = 0; i < sz; i++) {
            printf("%s", data_arr[i].arg1);
            for (int a = 0; a < data_arr[i].d_len; a++) {
                printf(" %s", data_arr[i].arg_arr[a]);
            }
            printf("\n");

            if (check_arg(data_arr[i].arg1, "!!")){
                printf("%s\n", last_command);
                break;
            } else if (check_arg(data_arr[i].arg1, "exit")) {
                should_run = 0;
                break;
            } else if (set_style(&data_arr[i].d_len, data_arr[i].arg_arr, &selected) != 0) {
                continue;
            }
            
            if (!selected) {
                int res = exec_fork(&data_arr[i]);
                if (res != 0) {
                    printf("An error occurred");
                }
            }
            
            if (selected) {
                if (pthread_create(&th[th_c], NULL, (void*) exec_fork, (void*) &data_arr[i]) != 0) {
                    fprintf(stderr, "Error pthread create %ld\n", th[th_c]);
                    exit(1);
                } else {
                    th_c++;
                }
            }
        }

        for (int i = 0; i < th_c; i++) pthread_join(th[i], NULL);
        for (int i = 0; i < th_c; i++) th[i] = 0;
        
        free(data_arr);

        if (is_file) exit(0);
    }
	return 0;
}
