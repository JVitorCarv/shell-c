CC = gcc

objects = shell.o

output: $(objects)
	$(CC) -o shell shell.c

.PHONY: clean

clean:
	rm $(objects) shell
