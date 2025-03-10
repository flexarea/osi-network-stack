#ifndef UTIL_H
#define UTIL_H

#include <sys/types.h>
#include <stdint.h>

char *binary_to_hex(void *data, ssize_t  n);
void *hex_to_binary(char *hex, ssize_t *bin_bytes);
int get_size(char *hex);
char upper_case(char c);
int non_hex(char x);
void handle_frame(char *data_as_hex);

typedef struct{
	uint8_t dest_addr[6];
	uint8_t src_addr[6];
	uint16_t type;
	uint8_t *data; //will point to malloced buffer
	uint32_t frame_check_sequence;
}frame_fields;

#endif
