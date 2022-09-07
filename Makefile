CC = gcc

objects = shell.o shell_pipes.o

output: $(objects)
	$(CC) -o shell shell.c -pthread
	$(CC) -o shell_pipes shell_pipes.c

.PHONY: clean

clean:
	rm $(objects) shell
