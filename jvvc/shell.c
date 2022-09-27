#include "func.h"

#define MAX_LINE 80 

/*_______________________________________________________*/
/*|                                                     |*/
/*|                       C SHELL                       |*/
/*|                    Vitor Carvalho                   |*/
/*|_____________________________________________________|*/

int main(int argc, char *argv[])
{
    if (argc > 2) {
        printf("Too many arguments were provided\n");
        return 0;
    }
    
    char *args[MAX_LINE/2 + 1];	
    int should_run = 1;		
    char* style[2] = {"seq", "par"};
    int selected = 0;

    char* last_cmd = (char*) malloc(MAX_LINE * sizeof(char));
    last_cmd = "!!";
    int has_cmd = 0;

    char line_arr[40][128];
    memset(line_arr, '\0', sizeof(line_arr));
    int is_file = 0, file_c = 0, line_c = 0;

    if (argc == 2) {
        is_file = 1;
        FILE* file = fopen(argv[1], "r");
        if (file == NULL) {
            printf("Could not find %s\n", argv[1]);
            exit(1);
        }
        char input[MAX_LINE * (MAX_LINE/2+1)]; 
        char line[128];

        while (line_c < 40 && fgets(line, sizeof(line), file)) {
            rmv_n(line);
            strncpy(line_arr[line_c], line, sizeof(line));
            line_c = line_c + 1;
        }
        fclose(file);
    }

    int spot_bckgnd = 0;

    while (should_run) {

        char* cmd_arr[MAX_LINE/2 + 1];
        int cmd_len = 0;

        if (!is_file) {
            char* input = (char*)malloc(MAX_LINE * sizeof(char*));
            memset(input, '\0', sizeof(char*)*MAX_LINE);
            get_input(style[selected], input); 
            get_args(&cmd_len, cmd_arr, input, ";");
        }
        else {
            get_args(&cmd_len, cmd_arr, line_arr[file_c], ";"); 
        }

        arg_data* data_arr = (arg_data*) malloc(cmd_len * sizeof(arg_data));
        int sz = 0;

        pthread_t th[cmd_len];
        int th_c = 0;

        char* cp_cmd = (char*) malloc(sizeof(char*));
        memset(cp_cmd, '\0', sizeof(cp_cmd));
        int copy = 0;

        for (int i = 0; i < cmd_len; i++) {
            if (is_blank(cmd_arr[i]))
                continue;
            
            char* args[MAX_LINE/2 + 1];
            memset(args, '\0', MAX_LINE/2 + 1);
            int arg_len = 0;
            
            /* Set last command */
            if (strlen(cmd_arr[i]) >= 2 && strlen(cmd_arr[i]) <= 40) {
                char temp[41];
                memset(temp, '\0', 41);
                int temp_c = 0;

                /* Creates a temp copy of the cmd_arr[i] to better make comparisons 
                and manipulations, since cmd_arr[i] is of char* type */
                strncpy(temp, cmd_arr[i], strlen(cmd_arr[i]));
                for(int w = 0; w < strlen(cmd_arr[i]); w++) {
                    if (isspace(cmd_arr[i][w]) == 0) {
                        temp[temp_c] = cmd_arr[i][w];
                        temp_c++;
                    }
                }

                /* Overwrite command */
                if (check_arg(temp, "!!")) {
                    cmd_arr[i] = last_cmd;
                } 
                cp_cmd = malloc(41 * sizeof(char));
                memset(cp_cmd, '\0', 41);
                strncpy(cp_cmd, cmd_arr[i], strlen(cmd_arr[i]));
                copy = 1;
            }
            
            arg_data* ad = (arg_data*) malloc(sizeof(arg_data));

            /* Background case */
            if (check_has(cmd_arr[i], '&')) {
                printf("Achei o &\n");
                spot_bckgnd = 1;
                cmd_arr[i] = strtok(cmd_arr[i], "&");
            }

            int is_redir = 0;

            /* Redirection append case */
            if (strstr(cmd_arr[i], ">>")) is_redir = 2;
            if (is_redir == 2 && strlen(cmd_arr[i]) >= 3) {
                ad->redir_type = 2;
                get_redir_args(cmd_arr[i], args, &arg_len, ">>");
                if (arg_len < 2) continue;
                
                get_redir_data(ad, args);
            }

            /* Redirection write case */
            if (!is_redir) is_redir = check_has(cmd_arr[i], '>');
            if (is_redir == 1 && strlen(cmd_arr[i]) >= 3) {
                ad->redir_type = 1;
                get_redir_args(cmd_arr[i], args, &arg_len, ">");
                if (arg_len < 2) continue;
                
                get_redir_data(ad, args);
            }

            /* Redirection to exe */
            if (!is_redir) {
                if(check_has(cmd_arr[i], '<')) {
                    is_redir = 3;
                }
            }
            if (is_redir == 3 && strlen(cmd_arr[i]) >= 3) {
                ad->redir_type = 3;
                get_redir_args(cmd_arr[i], args, &arg_len, "<");
                if (arg_len < 2) continue;
                
                get_redir_data(ad, args);
            }

            /* Pipe case */
            int is_pipe = check_has(cmd_arr[i], '|');
            pipe_arg_data* pipe_ad = (pipe_arg_data*) malloc(sizeof(pipe_arg_data));

            if (!is_redir && is_pipe && strlen(cmd_arr[i]) >= 3) {
                get_redir_args(cmd_arr[i], args, &arg_len, "|");
                if (spot_bckgnd) {
                    spot_bckgnd = 0;
                }
                if (arg_len < 2) continue;

                get_pipe_data(pipe_ad, args);
            }
            
            /* Normal command case */
            if(!is_pipe && !is_redir) {
                get_args(&arg_len, args, cmd_arr[i], " ");

                /* Creates new arg_data struct to be stored in data_arr */
                arg_data* ad = (arg_data*) malloc(sizeof(arg_data));
                memset(ad->arg_arr, '\0', MAX_LINE); // Clears trash
                get_data_arr(ad, arg_len, args, data_arr, sz);
                sz = sz + 1;
                if (spot_bckgnd) {
                    data_arr[sz-1].is_bckgnd = 1;
                    spot_bckgnd = 0;
                }

                clear_args(arg_len, args);
            }
            
            /* Custom commands handler */
            if (!is_pipe && !is_redir) {
                if (!has_cmd && check_arg(data_arr[sz-1].arg1, "!!")){
                    printf("No commands\n");
                    continue;
                } else if (check_arg(data_arr[sz-1].arg1, "exit")) {
                    should_run = 0;
                    memset(cmd_arr, '\0', cmd_len);
                    free(data_arr);
                    exit(0);
                } else if (set_style(&data_arr[sz-1].d_len, data_arr[sz-1].arg_arr, &selected) != 0) {
                    continue;
                }
            }
            
            /* Sequential approach */
            if (!selected) {
                if (is_pipe) {
                    int res = exec_pipe(pipe_ad);
                    if (res > 0) {
                        printf("An error occurred while executing pipe\n");
                    }
                } else if (!is_redir){
                    exec_fork(&data_arr[sz-1]);
                } else if (is_redir) {
                    exec_redir(ad);
                }
            }
            
            /* Parallel approach */
            if (selected) {
                if (is_pipe) {
                    th_c+=2;
                    int fd[2];
                    if (pipe(fd) < 0) {
                        fprintf(stderr, "Pipe creation failed");
                        return 1;
                    }

                    pid_t pid1 = fork();

                    if (pid1 == 0) {
                        close(STDOUT_FILENO);
                        close(fd[STDIN_FILENO]);
                        dup(fd[STDOUT_FILENO]);

                        int res1 = execvp(pipe_ad->arg1, pipe_ad->arg_arr1);
                        if (res1 < 0) {
                            fprintf(stderr, "Error while trying to execute %s: %s\n", pipe_ad->arg1, strerror(errno));
                            kill(getpid(), SIGKILL); /* Kills the process, so it doesn't keep existing */
                        }
                    }

                    if (pid1 < 0) {
                        printf("Error while forking in pid1, exec_pipe()\n");
                        return 1;
                    }

                    pid_t pid2 = fork();

                    if (pid2 == 0) {
                        close(STDIN_FILENO);
                        close(fd[STDOUT_FILENO]);
                        dup(fd[STDIN_FILENO]);
                        
                        int res2 = execvp(pipe_ad->arg2, pipe_ad->arg_arr2);
                        if (res2 < 0) {
                            fprintf(stderr, "Error while trying to execute %s: %s\n", pipe_ad->arg2, strerror(errno));
                            kill(getpid(), SIGKILL); /* Kills the process, so it doesn't keep existing */
                        }
                    }

                    if (pid2 < 0) {
                        printf("Error while forking in pid2, exec_pipe()\n");
                        return 1;
                    }

                    close(fd[STDIN_FILENO]);
                    close(fd[STDOUT_FILENO]);
                    
                    /* Waits for both processes */
                    //waitpid(pid1, NULL, 0);
                    //waitpid(pid2, NULL, 0);
                    
                } else if (!is_redir){
                    th_c++;

                    pid_t pid = fork();

                    if (pid < 0) {
                        printf("Fork failed\n");
                        free(&data_arr[sz-1]);
                    } else if (pid == 0) {
                        /* Debug print to prove that is executing in parallel processes */
                        //printf("[son] pid %d from [parent] pid %d\n",getpid(),getppid());
                        int code = execvp(data_arr[sz-1].arg1, data_arr[sz-1].arg_arr);
                        if (data_arr[sz-1].is_bckgnd == 1) {
                            setpgid(0, 0);
                        }
                        if (code < 0) {
                            fprintf(stderr, "Error while trying to execute %s: %s\n", data_arr[sz-1].arg1, strerror(errno));
                            kill(getpid(), SIGKILL); /* Kills the process, so it doesn't keep existing */
                        }
                        exit(0);
                    }
                } else if (is_redir) {
                    th_c++;
                    
                    pid_t pid = fork();

                    if (is_blank(ad->filename)) {
                        printf("File name must not be blank\n");
                        exit(0);
                    }

                    if (pid < 0) {
                        printf("Fork failed\n");
                        free(ad);
                    } else if (pid == 0) {
                        if (ad->redir_type == 1) {// >
                            int fd = open(ad->filename, O_CREAT | O_WRONLY, 0600);
                            if (fd < 0) printf("Error while trying to open %s", ad->filename);
                            dup2(fd, STDOUT_FILENO);
                            close(fd);
                        } else if (ad->redir_type == 2) {// >>
                            int fd = open(ad->filename, O_CREAT | O_WRONLY | O_APPEND, 0600);
                            if (fd < 0) printf("Error while trying to open %s", ad->filename);
                            dup2(fd, STDOUT_FILENO);
                            close(fd);
                        } else if (ad->redir_type == 3) {// <
                            FILE* file = fopen(ad->filename, "r");
                            if (file == NULL) {
                                printf("Could not find %s\n", ad->filename);
                                kill(getpid(), SIGKILL);
                                exit(0);
                            }
                            char* input = (char*)malloc(MAX_LINE * sizeof(char*));
                            memset(input, '\0', sizeof(char*)*MAX_LINE);

                            get_finput(file, input);
                            get_args(&ad->d_len, ad->arg_arr, input, "\n"); // Separates cmds by \n
                        }

                        int code = execvp(ad->arg1, ad->arg_arr);
                        if (code < 0) {
                            fprintf(stderr, "Error while trying to execute %s: %s\n", ad->arg1, strerror(errno));
                        }
                        kill(getpid(), SIGKILL); /* Kills the process, so it doesn't keep existing */
                    }
                }
            }
        }

        for (int i = 0; i < th_c; i++) wait(NULL);

        //for (int i = 0; i < th_c; i++) pthread_join(th[i], NULL);
        memset(th, '\0', cmd_len);
        
        memset(cmd_arr, '\0', cmd_len);
        free(data_arr);

        if(copy && strlen(cp_cmd) >= 1) last_cmd = cp_cmd;

        if (is_file && file_c == line_c - 1) {
            if (line_c >= 40) printf("Limit of 40 lines exceeded\n");
            exit(0);
        }
        if (is_file) file_c += 1;
    }
	return 0;
}
