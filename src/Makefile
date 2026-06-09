CC     = gcc
CFLAGS = -Wall -Wextra

all: monprog

monprog: main.o reseau.o loader.o
	$(CC) $(CFLAGS) -o monprog main.o reseau.o loader.o

main.o: main.c reseau.h
	$(CC) $(CFLAGS) -c main.c

reseau.o: reseau.c reseau.h
	$(CC) $(CFLAGS) -c reseau.c

loader.o: loader.c reseau.h
	$(CC) $(CFLAGS) -c loader.c

clean:
	rm -f *.o monprog