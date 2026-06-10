CC     = gcc
CFLAGS = -Wall -Wextra

TARGET = simulateur
SRCS   = main.c reseau.c config.c trame.c simulateur.c
OBJS   = $(SRCS:.c=.o)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

re: clean $(TARGET)

.PHONY: clean re
