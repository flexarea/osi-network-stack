#include<stdio.h>
#include<stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

int main(int argc, char *argv[]){
	struct stat file_stat;	
	int fd;
	ssize_t file_size;
	ssize_t file_bytes;
	char *hex_data;

	if(file == NULL){
		perror("Unable to open file");
		exit(1);
	}

	//handle no file provided
	if (argc == 1){
		int max = 256;
		char read_buffer[256];
		int bytes = read(0, *read_buffer, sizeof(max)-1); 

		if(bytes > 0){
			read_buffer[max] = '\0';
		}else{
			printf("Error reading  input\n")
		}
		close(0);
	}else{
		//read file
		if((fd = open(argv[1], O_RDONLY))){
			return NULL;	
		}
		if(stat(argv[1], &file_stat) == -1){
			perror("stat");
		}
		file_size = file_stat.st_size;
		char file_buffer[file_size];
		file_bytes = read(fd, *file_buffer, file_size);
		hex_data = binary_to_hex(*file_buffer, file_bytes);
	}
	return 0;
	close(fd);
}
