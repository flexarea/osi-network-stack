#include<stdio.h>
#include<stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include "util.h"
#include <string.h>

int main(int argc, char *argv[]){
	struct stat file_stat;	
	int fd;
	char *file_buffer;
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
			read_buffer[bytes] = '\0';
			hex_data = binary_to_hex(read_buffer, bytes);
			if(hex_data == NULL){
				perror("binary_to_hex");
				return 1;
			}
			free(hex_data);
		}else{
			printf("Error reading  input\n");
			return 1;
		}
	}else{
		//read file
		if((fd = open(argv[1], O_RDONLY)) == -1){
			return 1;	
		}
		if(stat(argv[1], &file_stat) == -1){
			perror("stat");
			close(fd);
			return 1;
		}
		file_size = file_stat.st_size;
		file_buffer = malloc(file_size);

		if(file_buffer == NULL){
			perror("malloc");
			close(fd);
			return 1;
		}

		if((file_bytes = read(fd, file_buffer, file_size)) == -1){
			perror("read");
			free(file_buffer);
			close(fd);
			return 1;
		}
		hex_data = binary_to_hex(file_buffer, file_bytes);
		if(hex_data == NULL){
			perror("binary_to_hex");
			free(file_buffer);
			close(fd);
			return 1;
		}
		if((bytes_written = write(1, hex_data, strlen(hex_data))) == -1){
			perror("write");
			free(file_buffer);
			free(hex_data);
			close(fd);
			return 1;
		}
		free(file_buffer);
		free(hex_data);
		close(fd);
	}
	return 0;
}
