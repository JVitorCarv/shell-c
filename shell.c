#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE 80 /* 80 chars per line, per command */


int exec_fork(char* command, char** arg_arr) {

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

void get_args(int* arg_len, char** args, char* input, char* sep) {
    // Updates len, changes array
    // Uses input string and separates based on sep
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

int set_style(int* arg_len, char** args, int* selected) {
    int len = strlen(args[0]);
    if (len >= 5) {
        if (strncmp("style", args[0], 5) == 0) {
            if (strcmp("sequential", args[1]) == 0) {
                *selected = 0;
                return 1;
            } else if (strcmp("parallel", args[1]) == 0) {
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

int verify_exit(char* arg) {
    if (strlen(arg) >= 4) {
        if (strncmp("exit", arg, 4) == 0) return 1;
    }
    return 0;
}

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

int has_blank(int arg_len, char** cmd_arr) {
    for(int i = 0; i < arg_len; i++) {
        if (verify_blank(cmd_arr[i])) {
            return i;
        }
    }
    return 0;
}


int main(int argc, char *argv[])
{
    // Executes ??????
    if (argc > 1) {
        char* arg_arr[argc];
        
        // Copies argv arguments to arg_arr
        for (int i = 0; i < argc-1; i++) {  
            arg_arr[i] = argv[i+1];
        }

        int res = exec_fork(argv[1], arg_arr);
    }

    char *args[MAX_LINE/2 + 1];	/* command line has max of 40 arguments */
    int should_run = 1;		/* flag to help exit program*/
    char* style[2] = {"seq", "par"};
    int selected = 0;

    while (should_run) {
        printf("jvvc %s> ", style[selected]);
            fflush(stdout);

        char* input = malloc(MAX_LINE);      
        fgets(input, MAX_LINE, stdin);

        // Remove \n
        int len = strlen(input);
        if (isspace(input[len-1]) != 0) {
            input[len-1] = 0;
        }
        
        //Separates commands by ;
        char* cmd_arr[MAX_LINE/2 + 1];
        int cmd_len = 0;

        get_args(&cmd_len, cmd_arr, input, ";");

        // executes for every different command, sequential
        if (!selected) {
            for (int i = 0; i < cmd_len; i++) {
                char* args[MAX_LINE/2 + 1]; //maybe change args to another name later
                int arg_len = 0;

                if (verify_blank(cmd_arr[i])) {
                    continue;
                }                 

                get_args(&arg_len, args, cmd_arr[i], " ");

                if (verify_exit(args[0])) {
                    should_run = 0;
                    break; 
                } else if (set_style(&arg_len, args, &selected) != 0) { // Change this later!!!!!!!!!!!! Change this later!!!!!!!!!!!! Change this later!!!!!!!!!!!! Change this later!!!!!!!!!!!!
                    clear_args(arg_len, args);
                    continue;
                }

                int res = exec_fork(args[0], args);

                clear_args(arg_len, args);
            }
        } else if (selected) {
            printf("Parallel not yet implemented\n");
            char* args[MAX_LINE/2 + 1]; //maybe change args to another name later
            int arg_len = 0;
            
            if (verify_exit(args[0])) {
                should_run = 0;
                break; 
            } else if (set_style(&arg_len, args, &selected) != 0) { // Change this later!!!!!!!!!!!! Change this later!!!!!!!!!!!! Change this later!!!!!!!!!!!! Change this later!!!!!!!!!!!!
                clear_args(arg_len, args);
                continue;
            }
        }
        
        clear_args(cmd_len, cmd_arr);
    }
	return 0;
}
