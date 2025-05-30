#include<stdio.h>
#include<stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include "util.h"
#include <string.h>
#include <signal.h>
#include <errno.h>


int main(int argc, char *argv[]){
	struct stat file_stat;	
	int fd;
	char *file_buffer;
	//ssize_t file_size;
	//ssize_t file_bytes;
	char *hex_data;
	ssize_t bytes_written;
	char *stdin_buffer;
	ssize_t bytes;

	//handle no file provided
	if (argc == 1){
		int max = 16;
		stdin_buffer = malloc(sizeof(max));
		ssize_t byte_counter = 0;
		char temp_buffer[16];

		// process running here
		while(1){
			bytes = read(0, stdin_buffer, max);
			if(bytes > 0){
				//copy read bytes into byte-counter
				for (ssize_t i=0; i < bytes; i++){
					temp_buffer[byte_counter++] = stdin_buffer[i];

					if(byte_counter == 16){
						hex_data = binary_to_hex(temp_buffer, 16); //convert binary to hex
						if(hex_data == NULL){
							perror("binary_to_hex");
							return 1;
						}
						//write data to stdout
						if((bytes_written = write(1, hex_data, strlen(hex_data))) == -1){ 
							perror("write");
							free(stdin_buffer);
							free(hex_data);
							return 1;
						}
						//free buffers to continue reading
						free(hex_data);
						//reset 
						byte_counter = 0;
					}
				}
			} else if (bytes == 0){
				break;
			} else if(errno == EINTR){
				continue;
			}else{
				perror("read");
				free(stdin_buffer);
				return 1;
			}
		}
		//flush buffer to stdout
		if(byte_counter > 0){
			hex_data = binary_to_hex(temp_buffer, byte_counter);
			if(hex_data == NULL){
				perror("binary_to_hex");
				free(stdin_buffer);
				return 1;
			}
			if(write(1, hex_data, strlen(hex_data)) == -1){
				perror("write");
				free(stdin_buffer);
				free(hex_data);
				return 1;
			}
			free(hex_data);

		}
		free(stdin_buffer);
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
		int max  = 1024;
		ssize_t byte_counter = 0;
		//file_size = file_stat.st_size;
		// file_buffer = malloc(file_size); //old code
		file_buffer = malloc(max);
		char temp_buffer[16];
	
		//new code start here
		while(1){
		bytes = read(fd, file_buffer, max);
			if(bytes > 0){
				//copy read bytes into byte-counter
				ssize_t i = 0;
				for (; i < bytes; i++){
					temp_buffer[byte_counter++] = file_buffer[i];

					if(byte_counter == 16){
						hex_data = binary_to_hex(temp_buffer, 16); //convert binary to hex
						if(hex_data == NULL){
							perror("binary_to_hex");
							return 1;
						}
						//write data to stdout
						if((bytes_written = write(1, hex_data, strlen(hex_data))) == -1){ 
							perror("write");
							free(hex_data);
							return 1;
						}
						//free buffers to continue reading
						free(hex_data);
						//reset 
						byte_counter = 0;
					}
				}
			} else if (bytes == 0){
				break;
			}else{
				perror("read");
				free(file_buffer);
				return 1;
			}
		}
		//flush buffer to stdout
		if(byte_counter > 0){
			hex_data = binary_to_hex(temp_buffer, byte_counter);
			if(hex_data == NULL){
				perror("binary_to_hex");
				free(file_buffer);
				return 1;
			}
			if(write(1, hex_data, strlen(hex_data)) == -1){
				perror("write");
				free(file_buffer);
				free(hex_data);
				return 1;
			}
			free(hex_data);

		}
	//new code ends here

		free(file_buffer);
		close(fd);
	}
	return 0;
}

