#include<stdio.h>
#include<stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include "util.h"

int main(int argc, char *argv[]){
	struct stat file_stat;	
	int fd;
	ssize_t file_size;
	ssize_t file_bytes;
	char *hex_data;
	ssize_t bytes_written;

	//handle no file provided
	if (argc == 1){
		int max = 256;
		char read_buffer[max];
		int bytes = read(0, read_buffer, max-1); 

		if(bytes > 0){
			read_buffer[max-1] = '\0';
		}else{
			printf("Error reading  input\n");
		}
		close(0);
	}else{
		//read file
		if((fd = open(argv[1], O_RDONLY)) == -1){
			return 1;	
		}
		if(stat(argv[1], &file_stat) == -1){
			perror("stat");
		}
		file_size = file_stat.st_size;
		char file_buffer[file_size];
		if((file_bytes = read(fd, file_buffer, file_size)) == -1){
			return 1;
		}
		hex_data = binary_to_hex(*file_buffer, file_bytes);
		if((bytes_written = write(1, hex_data, file_bytes)) == -1){
			return 1;
		}
	}
	close(fd);
	return 0;
}
