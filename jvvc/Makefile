CC = gcc

objects = shell.o func.o

output: $(objects)
	$(CC) -o shell shell.c func.c

shell.o: func.h

func.o: func.h

.PHONY: clean

clean:
	rm $(objects) shell
