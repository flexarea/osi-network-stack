#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

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
		if(hex_counter % 16 == 0 && hex_counter != 0){
			buffer[j] = '\n'; //add new line
			j++;
		}

		//convert byte to hex
		int higher_nibble = (((char *) data) [i] >> 4) & 0xF;
		int lower_nibble  = ((char *) data) [i] & 0xF;

		//adding hex pair to buffer
		buffer[j] = (higher_nibble < 10) ? (higher_nibble + '0') : (higher_nibble - 10 + 'A');
		buffer[j+1] = (lower_nibble < 10) ? (lower_nibble + '0') : (lower_nibble - 10 + 'A');

		hex_counter += 1; // increment hex_

		if(i != n - 1){
			if(hex_counter % 16 == 0 && hex_counter != 0){
				j += 2;
			}else{

				buffer[j+2] = ' ';
				j += 3;
			}
		}else{
			j += 2;
		}
		i++;
	}

	buffer[j] = '\n';
	buffer[j+1] = '\0';
	return buffer;
}

void *hex_to_binary(char *hex, ssize_t *bin_bytes){
	int size = get_size(hex);	
	void *buffer = malloc(size);

	if(buffer == NULL){
		return NULL;
	}

	ssize_t i = 0;
	ssize_t counter = 0;
	char *t = hex; //copy hex

	while(*t != '\0'){
		if(isspace(*t)){
			t++;	
		}else{
			if(*(t+1) == '\0'){
				free(buffer);
				return NULL;
			}
			char  high = *t;
			char  low  = *(t+1);

			//check for non-hex char for both nibbles
			if(non_hex(high) || non_hex(low)){
				free(buffer);
				return NULL;
			}

			int higher_nibble = (high >= '0' && high <= '9') ? (high - '0') : (upper_case(high) + 10 - 'A');
			int lower_nibble  = (low >= '0' && low <= '9') ? (low - '0') : (upper_case(low) + 10 - 'A');

			((unsigned char *)buffer)[i] = (higher_nibble << 4) | lower_nibble; //store ascii representation of hex value in buffer	
			i++;
			counter++;
			t += 2;	
		}
	}
	*bin_bytes = counter;	
	return buffer;
}

int get_size(char *hex){
	char *t;
	int size = 0;
	for (t = hex; *t !='\0'; t++){
		if (!isspace(*t)){
			size++;
		}
	}
	return size;
}
char upper_case(char c){
	char c1 = c;
	c1 &= ~' ';
	return c1;
}
int non_hex(char x){
	if(!((x >= '0' && x <= '9') ||
				(x >='A' && x <= 'F') ||
				(x >= 'a' && x <='f'))){
		return 1;
	}
	return 0;
}
