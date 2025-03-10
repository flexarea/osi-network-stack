CC = gcc
CFLAGS = -Wall -g

all: hexread hexdump stack

hexread: hexread.o util.o
	$(CC) $(CFLAGS) hexread.o util.o -o hexread
hexread.o: hexread.c util.h
	$(CC) $(CFLAGS) -c hexread.c -o hexread.o
hexdump: hexdump.o util.o
	$(CC) $(CFLAGS) hexdump.o util.o -o hexdump
hexdump.o: hexdump.c util.h
	$(CC) $(CFLAGS) -c hexdump.c -o hexdump.o
stack: stack.o util.o
	$(CC) $(CFLAGS) stack.o util.o -o stack
stack.o: stack.c util.h
	$(CC) $(CFLAGS) -c stack.c -o stack.o
util.o: util.c util.h
	$(CC) $(CFLAGS) -c util.c -o util.o

.PHONY:clean
clean:
	rm -f *.o hexread hexdump
