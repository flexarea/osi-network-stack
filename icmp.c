#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "util.h"
#include "stack.h"
#include "cs431vde.h"
#include "crc32.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <poll.h>
#include <sys/random.h>

void handle_icmp(ssize_t len, struct frame_fields *frame_f, uint8_t *or_frame, struct ip_header *packet, struct packet_info *packet_inf, struct icmp *curr_icmp, struct interface *interface_list_, int transmitter_id){

	//copy ip header + 8 bytes payload
	memcpy(or_frame+42, or_frame+14, 28);

	//handle ICMP here
	packet->protocol = 1;	
	//identification number
	packet->total_length = htons(56);
	packet->ttl = 64;
	packet->identification = htons(rand_generator()); //should technically be random

	//update ip field
	memcpy(packet->dest_addr, packet->src_addr, 4);
	memcpy(packet->src_addr, interface_list_[transmitter_id].ip_addr, 4);
	//memcpy ip src addr to packet_inf->dest_ip_addr


	//add icmp field
	or_frame[34] = (uint8_t)curr_icmp->type;
	or_frame[35] = (uint8_t)curr_icmp->code;
	//initialize checksum to zero
	or_frame[36] = 0;
	or_frame[37] = 0;

	//unused
	memset(&or_frame[38], 0, 4);	

	//compute icmp checksum
	uint16_t new_checksum = ip_checksum(or_frame+34, 36);	
	or_frame[36] = (new_checksum >>8) & 0xFF;
	or_frame[37] = new_checksum & 0xFF;

	//update up length
	uint16_t new_total_length = htons(56);  //20+8+28
	memcpy(or_frame+16, &new_total_length, 2);

	//console output
	if(curr_icmp->type == 3){
		switch(curr_icmp->code){
			case 0:
				//network unreachable
				printf("dropping packet from %s to %s (no route)\n", packet_inf->src_ip_addr, packet_inf->dest_ip_addr);
	memcpy(packet_inf->src_ip_addr, packet_inf->dest_ip_addr, 4);
				break;
			case 1:
				//host unreachable
				printf("dropping packet from %s to %s (no ARP)\n", packet_inf->src_ip_addr, packet_inf->dest_ip_addr);
				break;
			default:
				//print something
				printf("unsupported icmp type\n");
		}
	}else{
		if(curr_icmp ->type == 11){
			//ttl excedeed
			printf("dropping packet from %s to %s (TTL exceeded)\n", packet_inf->src_ip_addr, packet_inf->dest_ip_addr);
			return;
		}else{
			printf("unsupported icmp type\n");
		}

	}
}

uint16_t rand_generator(){
	uint16_t buf;	

	if(getrandom(&buf, sizeof(buf), 0) == -1){
		perror("getrandom");
		return 0;
	}
	return buf;
}
