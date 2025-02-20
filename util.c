#include "util.h"
#include <stdio.h>
#include <stdlib.h>

int main(){
	char str[19] = "zzzzzzzzzzzzzzzzzz";
	char str2[] ="7A 7A 7A 7A";
	char *hex_data = binary_to_hex(&str, 19);
	ssize_t bin_bytes;
	char *binary_data = hex_to_binary(str2, &bin_bytes);	
	}
	printf("%s", binary_data);
	// printf("%s", hex_data);
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
		buffer[j] = (higher_nibble < 10) ? (higher_nibble + '0') : (higher_nibble - 10 + 'A');
		buffer[j+1] = (lower_nibble < 10) ? (lower_nibble + '0') : (lower_nibble - 10 + 'A');

		hex_counter += 1; // increment hex_
		buffer[j+2] = ' ';
		j += 3;
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

	char hex_1, hex_2;
	ssize_t i = 0;
	ssize_t counter = 0;
	char *t = hex; //copy hex

	while(*t != '\0'){
		if(*t == ' ' || *t == '\n'){
			t++;	
		}else{
			char  high = *t;
			char  low  = *(t+1);

			//check for non-hex char for both nibbles
			if(high < '0' || high > 'f'){
				free(buffer);
				return null;
			}
			if(low < '0' || low > 'F'){
				free(buffer);
				return NULL;
			}


			int higher_nibble = (high >= '0' && high <= '9') ? (high - '0') : (upper_case(high) + 10 - 'A');
			int lower_nibble  = (low >= '0' && low <= '9') ? (low - '0') : (upper_case(low) + 10 - 'A');

			buffer[i] = (higher_nibble >> 4) | lower_nibble; //store ascii representation of hex value in buffer	
			i++;
			counter++;
			t += 2;	
		}
	}
	*bin_bytes = counter;	
	buffer[i] = '\0';
	return buffer;
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
char upper_case(char c){
	char c1 = c;
	return (c < 'A') ? c : (c1 &= ~' ');
}

