CC     = gcc
CFLAGS = -Wall -Wextra

all: monprog

monprog: main.o reseau.o loader.o trame.o scheduler.o
	$(CC) $(CFLAGS) -o monprog main.o reseau.o loader.o trame.o scheduler.o

main.o: main.c reseau.h trame.h scheduler.h
	$(CC) $(CFLAGS) -c main.c

reseau.o: reseau.c reseau.h
	$(CC) $(CFLAGS) -c reseau.c

loader.o: loader.c reseau.h
	$(CC) $(CFLAGS) -c loader.c

trame.o: trame.c trame.h reseau.h
	$(CC) $(CFLAGS) -c trame.c

scheduler.o: scheduler.c scheduler.h reseau.h trame.h
	$(CC) $(CFLAGS) -c scheduler.c

clean:
	rm -f *.o monprog