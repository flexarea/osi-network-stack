#include "util.h"
#include <stdio.h>
#include <stdlib.h>

int main(){
	char str[19] = "AAAAAAAAAAAAAAAAAA";
	char *hex_data = binary_to_hex(str, 19);
	int i = 0;
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
char *binary_to_hex(void *data, int n){
	int n_line = (n /16) + 1; //calculate the number of \n to add to the buffer + last  \n
	int length = (n*3) + n_line; //considering pair representation per character 
	char *buffer = (char* ) malloc(length * sizeof(char));

	if(buffer == NULL){
		return NULL;
	}
	int i = 0; //data iterator
	int j = 0; //buffer iterator
	int hex_counter = 0;

	while(i < n ){
		if (((char *) data)[i] == '\0'){
			break;
		}
		int higher_nibble = (((char *) data) [i] >> 4) & 0xF;
		int lower_nibble  = ((char *) data) [i] & 0xF;

		if(hex_counter % 16 == 0 && hex_counter != 0){
			buffer[j] = '\n'; //add new line
			j++;
		}else{
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
	}

	buffer[j] = '\n';
	buffer[j+1] = '\0';
	return buffer;
}

