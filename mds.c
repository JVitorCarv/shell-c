#include <unistd.h>
#include <string.h>

struct command
{
  const char **argv;
};

int spawn_proc (int in, int out, struct command *cmd)
{
  pid_t pid;

  if ((pid = fork ()) == 0)
    {
      if (in != 0)
        {
          dup2 (in, 0);
          close (in);
        }

      if (out != 1)
        {
          dup2 (out, 1);
          close (out);
        }

      return execvp (cmd->argv [0], (char * const *)cmd->argv);
    }

  return pid;
}

int fork_pipes (int n, struct command *cmd)
{
  int i;
  pid_t pid;
  int in, fd [2];

  /* The first process should get its input from the original file descriptor 0.  */
  in = 0;

  /* Note the loop bound, we spawn here all, but the last stage of the pipeline.  */
  for (i = 0; i < n - 1; ++i)
    {
      pipe (fd);

      /* f [1] is the write end of the pipe, we carry `in` from the prev iteration.  */
        pid_t pid = fork();
        if ((pid == 0)
            {
            if (in != 0)
                {
                dup2 (in, 0);
                close (in);
                }

            if (out != 1)
                {
                dup2 (out, 1);
                close (out);
                }
            }
        }

      /* No need for the write end of the pipe, the child will write here.  */
      close (fd [1]);

      /* Keep the read end of the pipe, the next child will read from there.  */
      in = fd [0];
    }

  /* Last stage of the pipeline - set stdin be the read end of the previous pipe
     and output to the original file descriptor 1. */  
  if (in != 0)
    dup2 (in, 0);

  /* Execute the last stage with the current process. */
  return execvp (cmd [i].argv [0], (char * const *)cmd [i].argv);
}

int main ()
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

  return fork_pipes (4, cmd);
}