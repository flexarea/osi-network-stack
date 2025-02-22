#include<stdio.h>
#include<stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "util.h"
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

volatile sig_atomic_t sig_val = 1;

int main(int argc, char *argv[]){
	struct stat file_stat;	
	int fd;
	//ssize_t file_size;
	//ssize_t file_bytes;
	ssize_t bin_bytes;
	char *file_buffer;
	char *binary_data;
	ssize_t bytes_written;
	char *stdin_buffer;
	ssize_t bytes;

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
		int max = 1024;
		stdin_buffer = malloc(max);
		ssize_t byte_counter = 0;
		ssize_t valid_hex_counter = 0;
		char temp_buffer[48];

		// process running here
		while(sig_val){
			ssize_t bytes = read(0, stdin_buffer, 48);
			if(bytes > 0){
				//copy read bytes into byte-counter
				for (ssize_t i=0; i < bytes; i++){
					if(!isspace(stdin_buffer[i])){
						valid_hex_counter++;
					}
					temp_buffer[byte_counter++] = stdin_buffer[i];

					if(valid_hex_counter == 32){
						char null_terminated_buffer[byte_counter+1];
						memcpy(null_terminated_buffer, temp_buffer, byte_counter);
						null_terminated_buffer[byte_counter] = '\0';
						binary_data = hex_to_binary(null_terminated_buffer, &bin_bytes); //convert binary to hex

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
						write(1, "\n", 1);
						//free buffers to continuer reading
						free(binary_data);
						//reset 
						byte_counter = 0;
						valid_hex_counter = 0;
					}
				}
			} else if (bytes == 0){
				break;
			} else if (errno == EINTR){
				continue;
			}else{
				perror("read");
				free(stdin_buffer);
				return 1;
			}
		}
		//flush buffer to stdout
		if(byte_counter > 0){
			char null_terminated_buffer[byte_counter + 1];
			memcpy(null_terminated_buffer, temp_buffer, byte_counter);
			null_terminated_buffer[byte_counter] = '\0';
			binary_data = hex_to_binary(null_terminated_buffer, &bin_bytes);
			if(binary_data == NULL){
				perror("binary_to_hex");
				free(stdin_buffer);
				return 1;
			}
			if(write(1, binary_data, bin_bytes) == -1){
				perror("write");
				free(stdin_buffer);
				free(binary_data);
				return 1;
			}
			write(1, "\n", 1);
			free(binary_data);

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
		int max = 1024;
		ssize_t byte_counter = 0;

		file_buffer = malloc(max);
		char temp_buffer[48]; //if non-hex is included 16 valid hex aligned will never be more than 48 characters
		ssize_t valid_hex_counter = 0; //to include any character

		//new code start here
		while(1){
		bytes = read(fd, file_buffer, max);
			if(bytes > 0){
				//copy read bytes into byte-counter
				ssize_t i = 0;
				for (; i < bytes; i++){
					if(!isspace(file_buffer[i])){
						valid_hex_counter++;
					}
					temp_buffer[byte_counter++] = file_buffer[i];

					if(valid_hex_counter == 32){
						char null_terminated_buffer[byte_counter+1];
						memcpy(null_terminated_buffer, temp_buffer, byte_counter);
						null_terminated_buffer[byte_counter] = '\0';
						binary_data = hex_to_binary(null_terminated_buffer, &bin_bytes); //convert binary to hex

						if(binary_data == NULL){
							perror("hex_to_binary");
							free(file_buffer);
							free(binary_data);
							return 1;
						}
						//write data to stdout
						if((bytes_written = write(1, binary_data, bin_bytes)) == -1){ 
							perror("write");
							free(file_buffer);
							free(binary_data);
							return 1;
						}
						write(1, "\n", 1);
						//free buffers to continuer reading
						free(binary_data);
						//reset 
						byte_counter = 0;
						valid_hex_counter = 0;
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
			char null_terminated_buffer[byte_counter + 1];
			memcpy(null_terminated_buffer, temp_buffer, byte_counter);
			null_terminated_buffer[byte_counter] = '\0';
			binary_data = hex_to_binary(null_terminated_buffer, &bin_bytes);
			if(binary_data == NULL){
				perror("hex_to_binary");
				free(file_buffer);
				return 1;
			}
			if(write(1, binary_data, bin_bytes) == -1){
				perror("write");
				free(file_buffer);
				free(binary_data);
				return 1;
			}
			free(binary_data);

		}
	//new code ends here
		write(1, "\n", 1);
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
