#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "stack.h"
#include "cs431vde.h"
#include "crc32.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>


void handle_arp(struct frame_fields *frame_, uint8_t *or_frame, ssize_t len, struct packet_info *packet_inf, struct interface *interface_list_){
	struct arp *arp_ptr = (struct arp *) (or_frame + 14);
	//convert src address to ip str
	inet_ntop(AF_INET, &(arp_ptr->sender_ip), packet_inf->src_ip_addr, INET_ADDRSTRLEN);	
	inet_ntop(AF_INET, &(arp_ptr->target_ip), packet_inf->dest_ip_addr, INET_ADDRSTRLEN);	
	int found_interface = 0;
	int idx;
	printf("destination ip: %s\n", packet_inf->dest_ip_addr);
	if(ntohs(arp_ptr->opcode) == 1){

		//check if target ip match current interface
		for(int i=0; i<NUMBER_INTERFACES; i++){
			if(memcmp(interface_list_[i].ip_addr, arp_ptr->target_ip, 4) == 0){
				found_interface = 1;
				idx = i;
				break;
			}
		}

		if(!found_interface){
			printf("ignoring ARP from (not for me) %s\n", packet_inf->src_ip_addr);
			return;
		}

		//update dest/src addr in ethernet header
		memcpy(frame_->dest_addr, frame_->src_addr, 6);
		memcpy(frame_->src_addr, interface_list_[idx].mac_addr, 6);
		//update target field
		memcpy(arp_ptr->target_mac, frame_->dest_addr, 6);
		memcpy(arp_ptr->target_ip, arp_ptr->sender_ip, 4);
		//update sender field
		memcpy(arp_ptr->sender_ip, interface_list_[idx].ip_addr, 4);
		memcpy(arp_ptr->sender_mac, interface_list_[idx].mac_addr, 6);
		//set opcode to reply
		arp_ptr->opcode = htons(2);

		//compute checksum
		uint32_t crc = crc32(0, or_frame, len-4);	
		memcpy(or_frame+(len-4), &crc ,4);
		//send arp reply
		printf("about to send arp reply\n");
		send_ethernet_frame(interface_list_[idx].switch_[1], or_frame, len);
		printf("sent arp reply to %s\n", packet_inf->src_ip_addr);
		return;
	}
	printf("dropping ARP packet from %s\n", packet_inf->src_ip_addr);
}
