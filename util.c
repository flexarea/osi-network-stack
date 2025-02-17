#include "util.h"
#include <stdio.h>
#include <stdlib.h>

int main(){
	char *str[5] = "AAAA";
	char *hex_data = binary_to_hex(str, 4);
	int i = 0;
	while(hex_data[i] != '\0'){
		printf("%c\n", hex_data[i]);
		i++;
	}
	return 0;
}
char *binar_to_hex(void *data, int n){
	int length = (n*4); 
	char *buffer = (char* ) malloc(4 * sizeof(char) + 1);

	if(buffer == null){
		/*
		 * TODO: add error signal
		 * */
		return;
	}
	int i = 0; //data iterator
	int j = 0; //buffer iterator
	int new_line_checker = 0;
	while(data[i] != '\0'){
		int higher_nibble = ((*(data + i) >> 4) & 0xF);
		int lower_nibble  = *(data + i + 1) & 0xF;

		if(new_line_checker % 16 == 0){
			buffer[j] = '\n'; //add new line
			j++;
			new_line_checker = 0;
		}
		
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
		new_line_checker += 2; 
		buffer[j+2] = ' ';

		j += 3;
		i += 2;

	}

	buffer[j] = '\0';
	return buffer;
}

