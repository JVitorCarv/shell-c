#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>

static char *my_command0[] = {"cat", "commands.txt", NULL};
static char *my_command1[] = {"grep", "return", NULL};
static char *my_command2[] = {"sed", "s/^ *//g", NULL};

static char **const my_commands[] = {
  my_command0,
  my_command1,
  my_command2,
  NULL
};

int create_sub_process(char *const command[], int input_stream)
{
    int pipefd[2] = {-1, -1};
    pid_t fk;

    if (pipe(pipefd) < 0)
    {
        perror("pipe");
        close(input_stream);
        return -1;
    }

    if ((fk = fork()) < 0)
    {
        perror("fork");
        close(pipefd[0]);
        close(pipefd[1]);
        close(input_stream);
        return -1;
    }

    if (fk == 0)
    {
        close(pipefd[0]);

        close(0);
        dup(input_stream);
        close(input_stream);

        close(1);
        dup(pipefd[1]);
        close(pipefd[1]);

        execvp(command[0], command);
        perror("execvp");
        exit(1);
    }

    close(input_stream);
    close(pipefd[1]);

    return pipefd[0];
}

int main()
{
    int fd = dup(0);

    for (int i = 0; my_commands[i] != NULL; i++)
    {
        fd = create_sub_process(my_commands[i], fd); // replace my_commands[i] by whatever you need to execute - check: man execvp
        if (fd < 0)
        {
            exit(1);
        }
    }

    // Also adapt the following lines to whatever you want to do with last child results
    close(0);
    dup(fd);
    close(fd);

    execlp("cat", "cat", (char *)NULL);
    perror("execlp");

    return 1;
}