CC = gcc

objects = shell.o

output: $(objects)
	$(CC) -o shell shell.c -pthread

.PHONY: clean

clean:
	rm $(objects) shell
