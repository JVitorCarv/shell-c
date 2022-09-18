#include "func.h"

#define MAX_LINE 80 /* 80 chars per line, per command */


int main(int argc, char *argv[])
{
    if (argc > 2) {
        printf("Too many arguments were provided\n");
        return 0;
    }
    
    char *args[MAX_LINE/2 + 1];	/* command line has max of 40 arguments */
    int should_run = 1;		/* flag to help exit program*/
    char* style[2] = {"seq", "par"};
    int selected = 0;

    int is_file = 0;
    char* last_cmd = (char*) malloc(MAX_LINE * sizeof(char));
    last_cmd = "!!";
    int has_cmd = 0;

    char line_arr[40][128];
    memset(line_arr, '\0', sizeof(line_arr));

    int file_c = 0;
    int line_c = 0;

    if (argc == 2) {
        is_file = 1;
        FILE* file = fopen(argv[1], "r");
        if (file == NULL) {
            printf("Could not find %s\n", argv[1]);
            exit(1);
        }
        char input[MAX_LINE * (MAX_LINE/2+1)]; /* max of 40 lines */
            
        //char* line = (char*) malloc(MAX_LINE * sizeof(char*));
        char line[128];

        while (line_c < 40 && fgets(line, sizeof(line), file)) {
            rmv_n(line);
            strncpy(line_arr[line_c], line, sizeof(line));
            line_c = line_c + 1;
        }
        fclose(file);
        
        if (line_c >= 40) printf("Limit of 40 lines exceeded\n");
    }

    int spot_bckgnd = 0;

    while (should_run) {

        char* cmd_arr[MAX_LINE/2 + 1];
        int cmd_len = 0;

        if (is_file) {
            get_args(&cmd_len, cmd_arr, line_arr[file_c], ";"); // Separates cmds by ;
        }

        if (!is_file) {
            char* input = (char*)malloc(MAX_LINE * sizeof(char*));
            memset(input, '\0', sizeof(char*)*MAX_LINE);
            get_input(style[selected], input); // Get input
            get_args(&cmd_len, cmd_arr, input, ";"); // Separates cmds by ;
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
            
            // Set last command
            if (strlen(cmd_arr[i]) >= 2 && strlen(cmd_arr[i]) <= 40) {
                char temp[41];
                memset(temp, '\0', 41);
                int temp_c = 0;

                strncpy(temp, cmd_arr[i], strlen(cmd_arr[i]));
                for(int w = 0; w < strlen(cmd_arr[i]); w++) {
                    if (isspace(cmd_arr[i][w]) == 0) {
                        temp[temp_c] = cmd_arr[i][w];
                        temp_c++;
                    }
                }
                //printf("%d", temp_c);
                //printf("Printando temp array\n");
                /*
                for(int w = 0; w < temp_c; w++) {
                    printf("%c", temp[temp_c]);
                }*/

                if (check_arg(temp, "!!")) {
                    //printf("Sobrescrever comando\n");
                    cmd_arr[i] = last_cmd;
                } 
                cp_cmd = malloc(41 * sizeof(char));
                memset(cp_cmd, '\0', 41);
                strncpy(cp_cmd, cmd_arr[i], strlen(cmd_arr[i]));
                copy = 1;
            }
            
            arg_data* ad = (arg_data*) malloc(sizeof(arg_data));

            int is_redir = 0;

            if (strstr(cmd_arr[i], ">>")) {
                is_redir = 2;
            }
            
            // Redirection append case
            if (is_redir == 2 && strlen(cmd_arr[i]) >= 3) {
                ad->redir_type = 2;
                get_redir_args(cmd_arr[i], args, &arg_len, ">>");
                if (arg_len < 2) continue;
                
                get_redir_data(ad, args);
            }

            if (!is_redir) is_redir = check_has(cmd_arr[i], '>');

            // Redirection write case
            if (is_redir == 1 && strlen(cmd_arr[i]) >= 3) {
                ad->redir_type = 1;
                get_redir_args(cmd_arr[i], args, &arg_len, ">");
                if (arg_len < 2) continue;
                
                get_redir_data(ad, args);
            }

            if (!is_redir) {
                if(check_has(cmd_arr[i], '<')) {
                    is_redir = 3;
                }
            }

            // Redirection to exe
            if (is_redir == 3 && strlen(cmd_arr[i]) >= 3) {
                ad->redir_type = 3;
                get_redir_args(cmd_arr[i], args, &arg_len, "<");
                if (arg_len < 2) continue;
                
                get_redir_data(ad, args);
            }

            int is_pipe = check_has(cmd_arr[i], '|');
            pipe_arg_data* pipe_ad = (pipe_arg_data*) malloc(sizeof(pipe_arg_data));

            if (check_has(cmd_arr[i], '&')) {
                printf("Achei o &\n");
                spot_bckgnd = 1;
                cmd_arr[i] = strtok(cmd_arr[i], "&");
            }
            // Pipe case
            if (!is_redir && is_pipe && strlen(cmd_arr[i]) >= 3) {
                get_redir_args(cmd_arr[i], args, &arg_len, "|");
                if (spot_bckgnd) {
                    spot_bckgnd = 0;
                }
                if (arg_len < 2) continue;

                get_pipe_data(pipe_ad, args); // Parses the text to pipe_data
            }
            

            // Normal command case
            if(!is_pipe && !is_redir) {
                /* TEMPORARY TEMPORARY TEMPORARY TEMPORARY TEMPORARY TEMPORARY */

                //Separates the raw text into string arrays
                get_args(&arg_len, args, cmd_arr[i], " ");

                //Creates new arg_data struct to be stored in data_arr
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
            
            if (!is_pipe && !is_redir) {
                // Custom shell handler
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
            
            // Sequential approach
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
            
            // Parallel approach
            if (selected) {
                if (is_pipe) {
                    if (pthread_create(&th[th_c], NULL, (void*) exec_pipe, pipe_ad) != 0) {
                        fprintf(stderr, "Error pthread create %ld\n", th[th_c]);
                        exit(1);
                    } else {
                        th_c++;
                    }
                } else if (!is_redir){
                    if (pthread_create(&th[th_c], NULL, (void*) exec_fork, &data_arr[sz-1]) != 0) {
                        fprintf(stderr, "Error pthread create %ld\n", th[th_c]);
                        exit(1);
                    } else {
                        th_c++;
                    }
                } else if (is_redir) {
                    if (pthread_create(&th[th_c], NULL, (void*) exec_redir, ad) != 0) {
                        fprintf(stderr, "Error pthread create %ld\n", th[th_c]);
                        exit(1);
                    } else {
                        th_c++;
                    }
                }
            }
        }

        for (int i = 0; i < th_c; i++) pthread_join(th[i], NULL);
        memset(th, '\0', cmd_len);
        
        memset(cmd_arr, '\0', cmd_len);
        free(data_arr);

        if(copy && strlen(cp_cmd) >= 1) last_cmd = cp_cmd;

        if (is_file && file_c == line_c - 1) exit(0);
        if (is_file) file_c += 1;
    }
	return 0;
}
