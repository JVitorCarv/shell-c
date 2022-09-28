#include <stdio.h>

/* This file was used during the development of 
redirections to an executable. */
int main(int argc, char *argv[]) {
    printf("ARGC: %d\n", argc);
    for (int i = 0; i < argc; i++) {
        printf("ARGV[%d] = %s\n", i, argv[i]);
    }
}
