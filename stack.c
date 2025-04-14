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

const int TABLE_LENGTH = 2;
const int CACHE_LENGTH = 2;

const uint8_t eth_addr[] = {0x12,0x9f,0x41,0x0d,0x0e,0x63}; //Interface ethernet address
const uint8_t interface_ip[]={0x8c,0xe9,0x01,0x01};       //Interface IP address
struct table_r routing_table; 


struct table_r *table = (struct table_r *) malloc(TABLE_LENGTH*sizeof(struct table_r));
struct arp_cache *arp_cache_table = (struct arp_cache *) malloc(CACHE_LENGTH*sizeof(struct arp_cache));

/*interface routing table configuration*/
memcpy(table[0].dest, "\x00\x00\x00\x00",4);
memcpy(table[0].gateway, "\x0a\x03\x00\x01",4); //send to gateway (next hop)
memcpy(table[0].genmask, "\x00\x00\x00\x00",4);
memcpy(table[1].dest, "\x0a\x03\x00\x00",4);
memcpy(table[1].gateway, "\x00\x00\x00\x00",4); //send to local subnework
memcpy(table[1].genmask, "\xff\xff\x00\x00",4);

/*ARP cache configuration*/
memcpy(arp_cache_table[0].ip_addr, "\x0a\x03\x02\x01",4);
memcpy(arp_cache_table[0].mac_addr, "\x12\x9f\x41\x0d\x0e\x65",4); //send to gateway (next hop)

int eth_cmp(struct frame_fields *frame_f,  const uint8_t *mac_addr);

struct frame_flags cf_flag; //current frame flag
struct frame_fields frame_header;
ssize_t d_size;
uint32_t cur_cfs; //received cfs

int main(int argc, char *argv[]){
	interface_receiver(&frame_header, &cf_flag, &cur_cfs, &d_size, eth_addr);
	return 0;
}

void interface_receiver(struct frame_fields *frame_f, struct frame_flags *curr_frame, uint32_t *curr_check_sum, ssize_t *data_size, const uint8_t *mac_addr){

	int switch1_fds[2];
	int switch2_fds[2];
	char *data_as_hex;

	uint8_t frame[1600];
	ssize_t frame_len;
	int connect_to_remote_switch = 0;
	char *local_vde_cmd[] = {"vde_plug", "/tmp/net1.vde", NULL};
	char *local_vde_cmd2[] = {"vde_plug", "/tmp/net2.vde", NULL};
	char *remote_vde_cmd[] = {"ssh", "entuyenabo@weathertop.cs.middlebury.edu", "/home/entuyenabo/cs431/bin/vde_plug", NULL};

	char **vde_cmd = connect_to_remote_switch ? remote_vde_cmd : local_vde_cmd;
	char **vde_cmd2 = connect_to_remote_switch ? remote_vde_cmd : local_vde_cmd2;

	//connect to switch1
	if(connect_to_vde_switch(switch1_fds, vde_cmd) < 0){
		printf("Could not connect to switch, exiting.\n");
		exit(1);
	}
	//connect to switch2
	if(connect_to_vde_switch(switch2_fds, vde_cmd2) < 0){
		printf("Could not connect to switch, exiting.\n");
		exit(1);
	}


	//read a single frame from switch
	while((frame_len = receive_ethernet_frame(switch1_fds[0], frame)) > 0) {
		data_as_hex = binary_to_hex(frame, frame_len);


		//retrieve frame fields information
		frame_f = (struct frame_fields *)frame;
		handle_frame(data_as_hex, frame_len, frame_f,  curr_frame, data_size,  curr_check_sum, mac_addr, frame);

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
void handle_packet(ssize_t len, struct frame_fields *frame_f, uint8_t *or_frame, struct table_r routing_table_, struct *arp_cache_){
	//jump to beginning of ip header
	uint8_t *fields_ptr = or_frame + 14;
	//cast to ip struct (extract ip header fields)
	struct ip_header *packet = (struct ip_header *) fields_ptr;
	char ip_str[INET_ADDRSTRLEN];
	struct in_addr ip_addr, genmask_addr;
	int subnet_list[TABLE_LENGTH];
	int lg_pfx_idx = -1; //longest subnet prefix index
	int lg_prefix = -1;
	int arp_idx; 


	/* Test print ip addr from packet
	 printf("IP SRC Address: %s\n", ip_str);	
	 */

	/*routing table logic*/	

	//convert dest ip to bin
	inet_ntop(AF_INET, &(packet->dest_addr), ip_str, INET_ADDRSTRLEN);	

	if(inet_aton(ip_str, &ip_addr)){
		printf("Binary IP (network byte order): %u\n", ip_addr.s_addr);
	}else{
		printf("error converting ip to binary\n");
	}

	for(int i=0; i<TABLE_LENGTH; i++){
		char genmask_str[INET_ADDRSTRLEN]; 
		char dest_str[INET_ADDRSTRLEN];
		struct in_addr dest_addr, result;
		int curr_lg_prefix;

		//convert addresses to ip
		inet_ntop(AF_INET, routing_table_[i].genmask, genmask_str, INET_ADDRSTRLEN);
		inet_ntop(AF_INET, routing_table_[i].dest, dest_str, INET_ADDRSTRLEN);


		//convert to bin
		inet_aton(genmask_str, &genmask_addr);
		inet_aton(dest_str, &dest_addr);
		//bitwise-and
		result.s_addr = ip_addr.s_addr & genmask_addr;
		//check matching dest addr
		if(result.s_addr == dest.s_addr){
			curr_lg_prefix = __builtin_popcount(ntohl(dest.s_addr));
			if(curr_lg_prefix > lg_prefix){
				lg_prefix = curr_lg_prefix;
				lg_pfx_idx = i;
			}	
		}
	}

	if(lg_pfx_idx != -1){
		//forward logic
		//consult arp cache
		int found_mac = 0;
		for(int i=0; i<CACHE_LENGTH; i++){
			if((memcmp(arp_cache_[i].ip_addr, routing_table_[lg_pfx_idx], 4)) == 0){
				//cpy arp_cache_[i].mac_addr to frame new dest addr
				arp_idx = i;
				found_mac = 1;
			}	
		}
		//encapsulation code here
		//transmission code here
		if(!found_mac){
			//send arp request here
		}
	}
}


ssize_t* encapsulation(struct table_r routing_table_, struct frame_fields *frame_, uint8_t *arp_cache_, int arp_idx_, int lg_pfx_idx_, struct ip_header packet_, ssize_t len){
	/*encapsulation logic*/
	memcpy(frame_f->src_addr, ether_addr, 6);
	memcpy(frame_f->dest_addr, arp_cache_[arp_idx_].mac_addr, 6);
	memcpy(packet_->src_addr, interface_ip, 4);
	memcpy(packet_->dest_addr, routing_table_[lg_pfx_idx_].dest, 4);
	packet_->ttl--;
	//recalculate checksum
	packet->checksum = ip_checksum(packet_);	
	//recreate frame
	memcpy(frame, packet_+14, len - 14);
}



