CC=gcc
CFLAGS= -Wall -O0 -g3
LIBFLAGS=-lcommons -pthread

all:
	$(CC) $(CFLAGS) socket.c server.c $(LIBFLAGS) -o server
	$(CC) $(CFLAGS) socket.c client.c $(LIBFLAGS) -o client
