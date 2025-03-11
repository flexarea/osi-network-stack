#ifndef STACK_H
#define STACK_H

#include <sys/types.h>
#include <stdint.h>

typedef struct frame_fields{
	uint8_t dest_addr[6];
	uint8_t src_addr[6];
	uint16_t type;
}frame_fields;


typedef struct frame_flags{
	int is_broadcast;	
	int is_for_me;	
	int check_sum_match;	
	int valid_length ;	
	uint32_t rcv_check_sum;
}frame_flags;

void handle_frame(char *data_as_hex, ssize_t len, struct frame_fields *frame_f, struct frame_flags *curr_frame, ssize_t *data_size, uint32_t *curr_check_sum, const uint8_t *mac_addr, uint8_t *or_frame);

void interface_receiver(struct frame_fields *frame_f, struct frame_flags *curr_frame, uint32_t *curr_check_sum, ssize_t *data_size, const uint8_t *mac_addr);
#endif
