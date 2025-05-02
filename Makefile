CC = gcc
CFLAGS = -Wall -g

#remember to add stack here
all: hexread hexdump receiver sender sender2 stack binary_exp switch hub

hexread: hexread.o util.o
	$(CC) $(CFLAGS) hexread.o util.o -o hexread
hexread.o: hexread.c util.h
	$(CC) $(CFLAGS) -c hexread.c -o hexread.o
hexdump: hexdump.o util.o
	$(CC) $(CFLAGS) hexdump.o util.o -o hexdump
hexdump.o: hexdump.c util.h
	$(CC) $(CFLAGS) -c hexdump.c -o hexdump.o
stack: stack.o util.o arp.o icmp.o ip.o ethernet.o ip-encapsulation.o cs431vde.c crc32.c
	$(CC) $(CFLAGS) stack.o util.o arp.o ip.o icmp.o ethernet.o ip-encapsulation.o cs431vde.c crc32.c -o stack
stack.o: stack.c util.h
	$(CC) $(CFLAGS) -c stack.c -o stack.o
sender: sender.c cs431vde.c util.o
	$(CC) $(CFLAGS) sender.c cs431vde.c crc32.c util.o -o sender
receiver: receiver.c cs431vde.c util.o
	$(CC) $(CFLAGS) receiver.c cs431vde.c util.o -o receiver
binary_exp: ethernet-study/binary_exp.c
	$(CC) $(CFLAGS) ethernet-study/binary_exp.c -o binary_exp
switch: ethernet-study/switch.c
	$(CC) $(CFLAGS) ethernet-study/switch.c -o switch
hub: ethernet-study/hub.c
	$(CC) $(CFLAGS) ethernet-study/hub.c -o hub
util.o: util.c util.h
	$(CC) $(CFLAGS) -c util.c -o util.o
arp.o: arp.c stack.h
	$(CC) $(CFLAGS) -c arp.c -o arp.o
icmp.o: icmp.c stack.h icmp.h
	$(CC) $(CFLAGS) -c icmp.c -o icmp.o
ip.o: ip.c stack.h ip.h
	$(CC) $(CFLAGS) -c ip.c -o ip.o
ethernet.o: ethernet.c stack.h ethernet.h
	$(CC) $(CFLAGS) -c ethernet.c -o ethernet.o
ip-encapsulation.o: ip-encapsulation.c stack.h ip.h
	$(CC) $(CFLAGS) -c ip-encapsulation.c -o ip-encapsulation.o
sender2: sender2.c cs431vde.c util.o
	$(CC) $(CFLAGS) sender2.c cs431vde.c crc32.c util.o -o sender2


.PHONY:clean
clean:
	rm -f *.o hexread hexdump receiver sender stack binary_exp switch hub sender2
