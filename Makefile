CC = gcc
CFLAGS = -Wall -g

all: hexread hexdump

hexread: hexread.o util.o
	$(CC) $(CFLAGS) hexread.o util.o -o hexread
hexread.o: hexread.c util.h
	$(CC) $(CFLAGS) -c hexread.c -o hexread.o
hexdump: hexwrite.o util.o
	$(CC) $(CFLAGS) hexwrite.o util.o -o hexdump
hexdump.o: hexwrite.c util.h
	$(CC) $(CFLAGS) -c hexwrite.c -o hexdump.o
util.o: util.c util.h
	$(CC) $(CFLAGS) -c util.c -o util.o

.PHONY:clean
clean:
	rm -f *.o hexread hexdump
