CC = gcc

objects = shell.o

output: $(objects)
	$(CC) -o shell shell.c -pthread
	$(CC) -o redir_file redir_file.c

shell.o: func.h

.PHONY: clean

clean:
	rm $(objects) shell redir_file
