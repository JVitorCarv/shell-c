#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>  

#define MAX_LINE 80

typedef struct {
    char* arg1;
    char* arg_arr[MAX_LINE];
    int d_len;
    int redir_type;
    char filename[40];
    int is_bckgnd;
}arg_data;

typedef struct {
    char* arg1;
    char* arg2;
    char* arg_arr1[MAX_LINE];
    char* arg_arr2[MAX_LINE];
    int d_len1;
    int d_len2;
}pipe_arg_data;

void exec_fork(arg_data* data);

void get_args(int* arg_len, char** args, char* input, char* sep);

void clear_args(int arg_len, char** args);

int set_style(int* arg_len, char** args, int* selected);

int check_arg(char* arg, char* comp);

int is_blank(char* arg);

int check_has(char* str, char sep);

void rmv_n(char* str);

void get_input(char* style, char* input);

void get_finput(FILE* file, char* input);

void get_data_arr(arg_data* ad, int arg_len, char** args, arg_data* data_arr, int sz);

void get_redir_data(arg_data* ad, char** args);

void get_pipe_data(pipe_arg_data* pipe_ad, char** args);

int exec_pipe(pipe_arg_data* pipe_ad);

void get_redir_args(char* cmd, char** args, int* arg_len, char* sep);

void exec_redir(arg_data* data);