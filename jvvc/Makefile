CC = gcc

objects = shell.o func.o

output: $(objects)
	$(CC) -o shell shell.c func.c
	$(CC) -o redir_file redir_file.c

shell.o: func.h

func.o: func.h

.PHONY: clean

clean:
	rm $(objects) shell redir_file
