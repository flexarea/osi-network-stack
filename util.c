#include "util.h"
#include <stdio.h>
#include <stdlib.h>

int main(){
	char str[19] = "zzzzzzzzzzzzzzzzzz";
	char *hex_data = binary_to_hex(str, 19);
	/*
	while(hex_data[i] != '\0'){
		printf("%c\n", hex_data[i]);
		i++;
	}
	*/
	printf("%s", hex_data);
	free(hex_data);
	return 0;
}
char *binary_to_hex(void *data, ssize_t n){
	ssize_t n_line = (n /16) + 1; //calculate the number of \n to add to the buffer + last  \n
	ssize_t length = (n*3) + n_line + 1; //considering pair representation per character 
	char *buffer = (char* ) malloc(length);

	if(buffer == NULL){
		return NULL;
	}
	ssize_t i = 0; //data iterator
	ssize_t j = 0; //buffer iterator
	ssize_t hex_counter = 0;

	while(i < n ){
		if (((char *) data)[i] == '\0'){
			break;
		}

		if(hex_counter % 16 == 0 && hex_counter != 0){
			buffer[j] = '\n'; //add new line
			j++;
		}

		//convert byte to hex
		int higher_nibble = (((char *) data) [i] >> 4) & 0xF;
		int lower_nibble  = ((char *) data) [i] & 0xF;

		//adding hex pair to buffer
		if (higher_nibble < 10){
			buffer[j] = higher_nibble + '0'; 
		} else {
			buffer[j] = higher_nibble - 10 + 'A';
		}
		if (lower_nibble < 10){
			buffer[j+1] = lower_nibble + '0'; 
		} else {
			buffer[j+1] = lower_nibble - 10 + 'A';
		}

		hex_counter += 1; // increment hex_
		buffer[j+2] = ' ';
		j += 3;
		i++;
	}

	buffer[j] = '\n';
	buffer[j+1] = '\0';
	return buffer;
}
/*
void *hex_to_binary(char *hex, ssize_t *bin_bytes){
	int size = get_size(hex);	
	void *buffer = malloc(size);

	int i = 0;
	ssize_t counter = 0;

	while(hex[i] != '\0'){
		
	}
}

int get_size(char *hex){
	char *t;
	int size = 0;
	for (t = hex; *t !='\0'; t++){
		if (*t == ' ' || *t == '\n'){
			size++;
		}	
	}
	return size;
}
*/













