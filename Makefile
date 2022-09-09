CC = gcc

objects = shell.o

output: $(objects)
	$(CC) -o shell shell.c -pthread

shell.o: func.h

.PHONY: clean

clean:
	rm $(objects) shell
