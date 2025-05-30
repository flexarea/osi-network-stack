#ifndef ETHERNET_H
#define ETHERNET_H

//stack.h struct
struct interface;
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

void handle_frame(char *data_as_hex, ssize_t len, struct frame_fields *frame_f, struct frame_flags *curr_frame, ssize_t *data_size, uint32_t *curr_check_sum,  uint8_t *or_frame, struct interface *interface_list_, int *receiver);

int eth_cmp(struct frame_fields *frame_f,  struct interface *interface_list_, int *receiver);

#endif
