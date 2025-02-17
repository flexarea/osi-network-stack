#include <stdio.h>

int main(){
	char data  = 'z'; //7a
	int higher_nibble = ( data >> 4) & 0xF;
	int lower_nibble  = data & 0xF;

	unsigned char byte = (higher_nibble << 4) | lower_nibble;	
	printf("%c\n",byte);

	return 0;

}
