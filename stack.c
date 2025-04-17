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

const int TABLE_LENGTH = 3;
const int CACHE_LENGTH = 3;

const uint8_t eth_addr[] = {0x12,0x9f,0x41,0x0d,0x0e,0x63}; //Interface ethernet address
const uint8_t interface_ip[]={0x8c,0xe9,0x01,0x01};       //Interface IP address

int eth_cmp(struct frame_fields *frame_f,  const uint8_t *mac_addr);

struct frame_flags cf_flag; //current frame flag
struct frame_fields frame_header;
ssize_t d_size;
uint32_t cur_cfs; //received cfs

int main(int argc, char *argv[]){

	struct table_r *interface_routing_table = (struct table_r *) malloc(TABLE_LENGTH*sizeof(struct table_r));
	struct arp_cache *interface_arp_cache = (struct arp_cache *) malloc(CACHE_LENGTH*sizeof(struct arp_cache));
	/*interface routing routing_table configuration*/
	memcpy(interface_routing_table[0].dest, "\x01\x02\x00\x00",4);
	memcpy(interface_routing_table[0].gateway, "\x00\x00\x00\x00",4); //subnework 1 (1.2/16)
	memcpy(interface_routing_table[0].genmask, "\xff\xff\x00\x00",4);
	memcpy(interface_routing_table[1].dest, "\x01\x01\x00\x00",4);
	memcpy(interface_routing_table[1].gateway, "\x00\x00\x00\x00",4); //subnetwork 2 (1.3/16)
	memcpy(interface_routing_table[1].genmask, "\xff\xff\x00\x00",4);
	memcpy(interface_routing_table[2].dest, "\x01\x04\x00\x00",4);
	memcpy(interface_routing_table[2].gateway, "\x01\x04\x00\x01",4); //subnetwork 3 (1.4/16) (inactive interface)
	memcpy(interface_routing_table[2].genmask, "\xff\xff\x00\x00",4);

	/*ARP cache configuration*/
	memcpy(interface_arp_cache[0].ip_addr, "\x01\x03\x04\x04",4);
	memcpy(interface_arp_cache[0].mac_addr, "\x12\x9f\x41\x0d\x0e\x65",6); //send to gateway (next hop)
	interface_receiver(&frame_header, &cf_flag, &cur_cfs, &d_size, eth_addr, interface_routing_table, interface_arp_cache);
	free(interface_routing_table);
	free(interface_arp_cache);
	return 0;
}

void interface_receiver(struct frame_fields *frame_f, struct frame_flags *curr_frame, uint32_t *curr_check_sum, ssize_t *data_size, const uint8_t *mac_addr, struct table_r *routing_table, struct arp_cache *arp_cache){

	int switch1_fds[2];
	int switch2_fds[2];
	char *data_as_hex;

	uint8_t frame[1600];
	ssize_t frame_len;
	int connect_to_remote_switch = 0;
	char *local_vde_cmd[] = {"vde_plug", "/home/entuyenabo/cs432/cs431/tmp/net1.vde", NULL};
	//char *local_vde_cmd2[] = {"vde_plug", "/home/entuyenabo/cs432/cs431/tmp/net2.vde", NULL};
	char *remote_vde_cmd[] = {"ssh", "entuyenabo@weathertop.cs.middlebury.edu", "/home/entuyenabo/cs431/bin/vde_plug", NULL};

	char **vde_cmd = connect_to_remote_switch ? remote_vde_cmd : local_vde_cmd;
	//char **vde_cmd2 = connect_to_remote_switch ? remote_vde_cmd : local_vde_cmd2;

	//connect to switch1
	if(connect_to_vde_switch(switch1_fds, vde_cmd) < 0){
		printf("Could not connect to switch, exiting.\n");
		exit(1);
	}
	//connect to switch2
	/*
	   if(connect_to_vde_switch(switch2_fds, vde_cmd2) < 0){
	   printf("Could not connect to switch, exiting.\n");
	   exit(1);
	   }*/


	//read a single frame from switch
	while((frame_len = receive_ethernet_frame(switch1_fds[0], frame)) > 0) {
		data_as_hex = binary_to_hex(frame, frame_len);


		//retrieve frame fields information
		frame_f = (struct frame_fields *)frame;
		handle_frame(data_as_hex, frame_len, frame_f,  curr_frame, data_size,  curr_check_sum, mac_addr, frame);
		//jump to beginning of ip header
		uint8_t *fields_ptr = frame + 14;
		//cast to ip struct (extract ip header fields)
		struct ip_header *ip_packet = (struct ip_header *) fields_ptr;
		struct icmp curr_packet_icmp;
		curr_packet_icmp.type = 1;
		struct packet_info ip_packet_info;


		if(curr_frame->valid_length == 0){
			printf("ignoring %d-byte frame (short)\n", (int)frame_len);
			continue;
		}

		if(curr_frame->check_sum_match == 0){
			printf("ignoring %d-byte frame", (int)frame_len);
			printf("(bad fcs: got 0x%08x, expected 0x%08x)\n", curr_frame->rcv_check_sum, *curr_check_sum);
			continue;
		}

		if(curr_frame->is_broadcast){
			printf("received %d-byte broadcast frame from %02x %02x %02x %02x %02x %02x\n", (int)frame_len, 
					frame_f->src_addr[0],
					frame_f->src_addr[1],
					frame_f->src_addr[2],
					frame_f->src_addr[3],
					frame_f->src_addr[4],
					frame_f->src_addr[5]);
			continue;
		}

		if(curr_frame->is_for_me){
			if(ntohs(frame_f->type) == 2048){
				handle_packet(frame_len, frame_f, frame, ip_packet, &ip_packet_info, &curr_packet_icmp, routing_table, arp_cache);
				//check ip icmp
				if(!ip_packet_info.valid_length){ //actually change comparison here, it's wrong
					printf("somewhere here\n");
					printf("dropping packet from %s (wrong length)\n", ip_packet_info.src_ip_addr);
					continue;
				}

				if(!ip_packet_info.valid_checksum){
					printf("inside checksum check\n");
					printf("dropping packet from %s (bad IP header checksum)\n", ip_packet_info.src_ip_addr);
					continue;
				}	
				if(curr_packet_icmp.type == 3){
					switch(curr_packet_icmp.code){
						case 0:
							//network unreachable
							printf("dropping packet from %s to %s (no route)\n", ip_packet_info.src_ip_addr, ip_packet_info.dest_ip_addr);
							break;
						case 1:
							//host unreachable
							printf("dropping packet from %s to %s (no ARP)\n", ip_packet_info.src_ip_addr, ip_packet_info.dest_ip_addr);
							break;
						default:
							//print something
					}
					continue;
				}

				if(curr_packet_icmp.type == 11){
					//ttl excedeed
					printf("dropping packet from %s to %s (TTL exceeded)\n", ip_packet_info.src_ip_addr, ip_packet_info.dest_ip_addr);
					continue;
				}

			}
			printf("received %d-byte frame for me from %02x %02x %02x %02x %02x %02x\n", (int)frame_len, 
					frame_f->src_addr[0],
					frame_f->src_addr[1],
					frame_f->src_addr[2],
					frame_f->src_addr[3],
					frame_f->src_addr[4],
					frame_f->src_addr[5]);
			continue;
		}

		free(data_as_hex);
	}

	if(frame_len < 0) {
		perror("read");
		exit(1);
	}
}

void handle_frame(char *data_as_hex, ssize_t len, struct frame_fields *frame_f, struct frame_flags *curr_frame, ssize_t *data_size, uint32_t *curr_check_sum, const uint8_t *mac_addr, uint8_t *or_frame){

	//partition frame fields
	*data_size = len - 14 - 4; //record datasize
	*curr_check_sum = *(uint32_t *) ( or_frame + len - 4);

	if(memcmp(frame_f->dest_addr,"\xff\xff\xff\xff\xff\xff", 6) == 0){
		curr_frame->is_broadcast = 1;
	}else{
		curr_frame->is_broadcast = 0;
	}
	if(eth_cmp(frame_f, mac_addr) == 1){
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

int eth_cmp(struct frame_fields *frame_f,  const uint8_t *mac_addr){
	for(int i=0; i<6; i++){
		if(frame_f->dest_addr[i] != mac_addr[i]){
			return 0;
		}
	}	
	return 1;
}

//function to extract and process ip header fields
void handle_packet(ssize_t len, struct frame_fields *frame_f, uint8_t *or_frame, struct ip_header *packet, struct packet_info *packet_inf, struct icmp *curr_icmp, struct table_r *routing_table, struct arp_cache *arp_cache__){
	printf("Running code in handle_packet\n");


	struct in_addr ip_addr;
	int lg_pfx_idx = -1; //longest subnet prefix index
	int lg_prefix = -1;
	int arp_idx; 

	//convert dest address to ip str
	inet_ntop(AF_INET, &(packet->dest_addr), packet_inf->dest_ip_addr, INET_ADDRSTRLEN);	
	//convert src address to ip str
	inet_ntop(AF_INET, &(packet->src_addr), packet_inf->src_ip_addr, INET_ADDRSTRLEN);	

	packet_inf->valid_length = (ntohs(packet->total_length) > 20) ? 1 : 0;

	packet_inf->valid_checksum = (ip_checksum(or_frame + 14) == 0) ? 1 : 0;

	if(!packet_inf->valid_checksum) return;


	if(packet->ttl <= 1){
		curr_icmp->type = 11;
		curr_icmp->code = 0;
		return;
	}else{
		packet->ttl--;
	}


	//convert dest ip to bin
	if(!inet_aton(packet_inf->dest_ip_addr, &ip_addr)){
		printf("error converting ip to binary\n");
	}

	for(int i=0; i<TABLE_LENGTH; i++){
		char genmask_str[INET_ADDRSTRLEN]; 
		char dest_str[INET_ADDRSTRLEN];
		struct in_addr dest_addr, genmask_addr, result;
		int curr_lg_prefix;

		//convert addresses to ip
		inet_ntop(AF_INET, routing_table[i].genmask, genmask_str, INET_ADDRSTRLEN);
		inet_ntop(AF_INET, routing_table[i].dest, dest_str, INET_ADDRSTRLEN);

		//convert to bin
		inet_aton(genmask_str, &genmask_addr);
		inet_aton(dest_str, &dest_addr);
		//bitwise-and
		result.s_addr = ip_addr.s_addr & genmask_addr.s_addr;
		//check matching dest addr

		if(result.s_addr == dest_addr.s_addr){
			curr_lg_prefix = __builtin_popcount(ntohl(genmask_addr.s_addr));
			if(curr_lg_prefix > lg_prefix){
				lg_prefix = curr_lg_prefix;
				lg_pfx_idx = i;
			}	
		}
	}

	char *real_path;



	if(lg_pfx_idx != -1){
		if (memcmp(routing_table[lg_pfx_idx].gateway, "\x00\x00\x00\x00", 4) == 0 ){
			real_path = packet_inf->dest_ip_addr;
		}else{
			real_path = (char *)routing_table[lg_pfx_idx].gateway;
		}
		//forward logic
		//consult arp cache
		int found_mac = 0;
		for(int i=0; i<CACHE_LENGTH; i++){
			if(memcmp(arp_cache__[i].ip_addr, real_path, 4) == 0){
				arp_idx = i;
				found_mac = 1;
			}	
		}
		//encapsulation code here
		//transmission code here
		if(!found_mac){
			curr_icmp->type = 3;
			curr_icmp->code = 1;
			//send arp request here
		}
	}else{
		//network unreacheable
		curr_icmp->type = 3;
		curr_icmp->code = 0;
	}

}


void encapsulation(struct frame_fields *frame_, int arp_idx_, int lg_pfx_idx_, struct ip_header *packet_, ssize_t len, uint8_t *or_frame, struct arp_cache *arp_cache_, struct icmp *curr_icmp, uint16_t type_){
	/*encapsulation logic*/
	memcpy(frame_->src_addr, eth_addr, 6);

	//ICMP
	if(curr_icmp->type != 1){
		
	}
	//ARP
	if(type_ == 2054){
		
	}
	memcpy(frame_->dest_addr, arp_cache_[arp_idx_].mac_addr, 6);

	or_frame[22] = packet_->ttl;

	//recalculate checksum
	uint16_t new_checksum = ip_checksum(or_frame+24);	

	//recreate frame
	or_frame[24] = (new_checksum >>8) & 0xFF;
	or_frame[25] = new_checksum & 0xFF;

	//compute checksum
	uint32_t crc = crc32(0, or_frame, len-4);	
	memcpy(or_frame+(len-4), &crc ,4);
}


