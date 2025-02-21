#include<stdio.h>
#include<stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "util.h"
#include <fcntl.h>

int main(int argc, char *argv[]){
	struct stat file_stat;	
	int fd;
	ssize_t file_size;
	ssize_t file_bytes;
	ssize_t *bin_bytes;
	char *file_buffer;
	char *binary_data;
	ssize_t bytes_written;

	//handle no file provided (read stdin)
	if (argc == 1){
		int max = 256;
		char read_buffer[max];
		int bytes = read(0, read_buffer, max-1);

		if(bytes > 0){
			read_buffer[bytes] = '\0';
		}else{
			printf("Error reading  input\n");
		}
	}else{
		//read file
		if((fd = open(argv[1], O_RDONLY)) == -1){
			perror("open");
			close(fd);
			return 1;
		}
		if(stat(argv[1], &file_stat) == -1){
			perror("stat");
			close(fd);
		}
		file_size = file_stat.st_size;
		file_buffer = malloc(file_size);
		if((file_bytes = read(fd, file_buffer, file_size)) == -1){
			perror("write");
			free(file_buffer);
			close(fd);
			return 1;
		};
		binary_data = hex_to_binary(file_buffer, bin_bytes);
		if(binary_data == NULL){
			perror("hex_to_binary");
			free(file_buffer);
			close(fd);
			return 1;
		}
		if((bytes_written = write(1, binary_data, *bin_bytes)) == -1){
			perror("write");
			free(file_buffer);
			free(binary_data);
			close(fd);
			return 1;
		}
		free(file_buffer);
		free(binary_data);
		close(fd);
	}
	return 0;
}
