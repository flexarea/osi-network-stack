#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "stack.h"
#include "ethernet.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include "crc32.h"

void handle_frame(char *data_as_hex, ssize_t len, struct frame_fields *frame_f, struct frame_flags *curr_frame, ssize_t *data_size, uint32_t *curr_check_sum,  uint8_t *or_frame, struct interface *interface_list_, int *receiver){

	//partition frame fields
	*data_size = len - 14 - 4; //record datasize
	*curr_check_sum = *(uint32_t *) ( or_frame + len - 4);

	if(memcmp(frame_f->dest_addr,"\xff\xff\xff\xff\xff\xff", 6) == 0){
		curr_frame->is_broadcast = 1;
	}else{
		curr_frame->is_broadcast = 0;
	}
	if(eth_cmp(frame_f, interface_list_, receiver) == 1){
		curr_frame->is_for_me = 1;
	}else{
		curr_frame->is_for_me = 0;
	}


	//compute checksum
	uint32_t crc = crc32(0, or_frame, len-4);	
	curr_frame->rcv_check_sum = ntohl(crc);
	curr_frame->check_sum_match = (*curr_check_sum == crc) ? 1 : 0;
	curr_frame->valid_length = (*data_size < 46) ? 0 : 1;
}

int eth_cmp(struct frame_fields *frame_f, struct interface *interface_list_, int *receiver){
	for(int i=0; i<NUMBER_INTERFACES; i++){
		if(memcmp(frame_f->dest_addr, interface_list_[i].mac_addr, 6) == 0){
			*receiver = i;
			return 1;
		}
		/*
		if(frame_f->dest_addr[i] != mac_addr[i]){
			return 0;
		}
		*/
	}	
	return 0;
}
