#include<stdio.h>
#include<stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "util.h"
#include <fcntl.h>
#include <signal.h>

volatile sig_atomic_t sig_val = 1;

int main(int argc, char *argv[]){
	struct stat file_stat;	
	int fd;
	ssize_t file_size;
	ssize_t file_bytes;
	ssize_t bin_bytes;
	char *file_buffer;
	char *binary_data;
	ssize_t bytes_written;
	char *stdin_buffer;

	//signal handling
	struct sigaction sa;
	sa.sa_handler = handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	
	if(sigaction(SIGINT, &sa, NULL) == -1) {
		perror("sigaction");
		return 1;
	}

	//handle no file provided
	if (argc == 1){
		int max = 16;
		stdin_buffer = malloc(sizeof(max));
		ssize_t byte_counter = 0;
		char temp_buffer[16];

		// process running here
		while(sig_val){
			ssize_t bytes = read(0, stdin_buffer, max);
			if(bytes > 0){
				//copy read bytes into byte-counter
				for (ssize_t i=0; i < bytes; i++){
					temp_buffer[byte_counter++] = stdin_buffer[i];

					if(byte_counter == 16){
						//the only way too fix this is either by adding \0 at the end of the buffer
						//or stripping off the \n at the end of the buffer, which wouldn't make sense cause we don't do that for hexdump
						binary_data = hex_to_binary(temp_buffer, &bin_bytes); //convert binary to hex
						if(binary_data == NULL){
							perror("hex_to_binary");
							free(stdin_buffer);
							free(binary_data);
							return 1;
						}
						//write data to stdout
						if((bytes_written = write(1, binary_data, bin_bytes)) == -1){ 
							perror("write");
							free(stdin_buffer);
							free(binary_data);
							return 1;
						}
						//free buffers to continuer reading
						free(binary_data);
						//reset 
						byte_counter = 0;
					}
				}
			} else if (bytes == 0){
				break;
			}else{
				perror("read");
				free(stdin_buffer);
				return 1;
			}
		}
		free(stdin_buffer);
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
		file_buffer = malloc(file_size + 1);
		if((file_bytes = read(fd, file_buffer, file_size)) == -1){
			perror("write");
			free(file_buffer);
			close(fd);
			return 1;
		};
		file_buffer[file_bytes] = '\0';
		binary_data = hex_to_binary(file_buffer, &bin_bytes);
		if(binary_data == NULL){
			perror("hex_to_binary");
			free(file_buffer);
			close(fd);
			return 1;
		}
		if((bytes_written = write(1, binary_data, bin_bytes)) == -1){
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

void handler(int sig){
	if(sig == SIGINT){
		sig_val = 0;
	}
}
