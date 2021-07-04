CC = cc
CFLAGS = -Wall -Wextra -std=c11 -pedantic

cuss: main.c
	$(CC) $(CFLAGS) -o cuss main.c
