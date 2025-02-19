util: util.c util.h
	gcc -g -o util util.c
.PHONY:clean
clean:
	rm -f util
