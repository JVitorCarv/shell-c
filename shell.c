#include "func.h"

#define MAX_LINE 80 /* 80 chars per line, per command */


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
            memset(input, '\0', sizeof(char*)*MAX_LINE);
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
            memset(args, '\0', MAX_LINE/2 + 1);
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
                if (arg_len > 2) {
                    printf("This shell only supports one pipe at a time.\n"
                    "Executing first pipe...\n");
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
                    if (pthread_create(&th[th_c], NULL, (void*) exec_pipe, pipe_ad) != 0) {
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
