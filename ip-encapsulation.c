#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "stack.h"
#include "icmp.h"
#include "ethernet.h"
#include "crc32.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>


void encapsulation(struct frame_fields *frame_, struct ip_header *packet_, ssize_t *len, uint8_t *or_frame, uint8_t *dest_addr_, struct icmp *curr_icmp,  struct interface *interface_list_, int error_, struct packet_info *packet_inf, int transmitter_id){
	/*encapsulation logic*/
	if(error_){
		//src mac_addr is received interface id
		memcpy(frame_->dest_addr, frame_->src_addr, 6);
		memcpy(frame_->src_addr, interface_list_[transmitter_id].mac_addr, 6);
		//ICMP encapsulation
		handle_icmp(len, frame_, or_frame, packet_, packet_inf, curr_icmp, interface_list_, transmitter_id);
	}else{
		memcpy(frame_->src_addr, interface_list_[transmitter_id].mac_addr, 6);
		memcpy(frame_->dest_addr, dest_addr_, 6);
	}


	or_frame[22] = packet_->ttl;

	or_frame[24] = 0;
	or_frame[25] = 0;
	//recalculate ip checksum
	uint16_t new_checksum = ip_checksum(or_frame+14, 20);	

	//compute icmp checksum
	or_frame[24] = (new_checksum >>8) & 0xFF;
	or_frame[25] = new_checksum & 0xFF;

	//compute checksum
	uint32_t crc = crc32(0, or_frame, *len-4);	
	memcpy(or_frame+(*len-4), &crc ,4);
}

//function checks if an ip address matches one of local interfaces'
int is_interface(struct interface *interface_list, uint8_t *ip_addr){
	for(int i=0; i<NUMBER_INTERFACES; i++){
		if(memcmp(interface_list[i].ip_addr, ip_addr, 4) == 0){
			printf("found match in is_interface\n");
			return i;
		}
	}
	return -1;
}

