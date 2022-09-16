#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>

#define READ 0
#define WRITE 1

void pipe_ex(char ***cmd, int sz)
{
	int fd[2];
	pid_t pid;
	int res, save_read = READ, fd_saved = 0;

	for(int i = 0; i < sz; i++) {
		res = pipe(fd);
		if (res < 0) {
			printf("Error while creating pipe\n");
		}
	    printf("%s\n\n", cmd[i][0]); // debug print
		pid = fork();
		if (pid < 0) {
			printf("Error while forking\n");
		}
		else if (pid == 0) {
			dup2(fd_saved, 0);
			close(fd[0]);
			res = execvp(cmd[i][0], cmd[i]);
			if (res < 0) {
				fprintf(stderr, "Error while trying to execute %s: %s\n", cmd[i][0], strerror(errno));
				kill(getpid(), SIGKILL);
			}
		}
		else {
			wait(NULL);
			close(fd[1]);
			fd_saved = fd[0];
		}
	}
}

int main()
{
	char *ls[40];
	memset(ls, '\0', sizeof ls);
	ls[0] = "ls";
	ls[1] = "-l";

	char *sort[40];
	memset(sort, '\0', sizeof sort);
	sort[0] = "sort";
	sort[1] = "-k";
	sort[2] = "5";

	char **cmd[10];
	memset(cmd, '\0', sizeof cmd);
	cmd[0] = ls;
	cmd[1] = sort;

	int sz = 2;
	/*for (int i = 0; i < sz; i++) {
		printf("%s", cmd[i][0]);
	}*/

	pipe_ex(cmd, sz);
	return (0);
}