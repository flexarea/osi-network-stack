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

const uint8_t eth_addr[] = {0x12,0x9f,0x41,0x0d,0x0e,0x63}; //Interface ethernet address

int eth_cmp(struct frame_fields *frame_f,  const uint8_t *mac_addr);

struct frame_flags cf_flag; //current frame flag
struct frame_fields frame_header;
ssize_t d_size;
uint32_t cur_cfs; //received cfs
				  //
int main(int argc, char *argv[]){
	interface_receiver(&frame_header, &cf_flag, &cur_cfs, &d_size, eth_addr);
	return 0;
}

void interface_receiver(struct frame_fields *frame_f, struct frame_flags *curr_frame, uint32_t *curr_check_sum, ssize_t *data_size, const uint8_t *mac_addr){

	int fds[2];
	char *data_as_hex;

	uint8_t frame[1600];
	ssize_t frame_len;
	int connect_to_remote_switch = 0;
	char *local_vde_cmd[] = {"vde_plug", NULL};
	char *remote_vde_cmd[] = {"ssh", "entuyenabo@weathertop.cs.middlebury.edu", "/home/entuyenabo/cs431/bin/vde_plug", NULL};

	char **vde_cmd = connect_to_remote_switch ? remote_vde_cmd : local_vde_cmd;

	//connect to switch
	if(connect_to_vde_switch(fds, vde_cmd) < 0){
		printf("Could not connect to switch, exiting.\n");
		exit(1);
	}


	//read a single frame from switch
	while((frame_len = receive_ethernet_frame(fds[0], frame)) > 0) {
		data_as_hex = binary_to_hex(frame, frame_len);


		//retrieve frame fields information
		frame_f = (struct frame_fields *)frame;
		handle_frame(data_as_hex, frame_len, frame_f,  curr_frame, data_size,  curr_check_sum, mac_addr, frame);

		printf("Recorded payload size: %d\n", (int)*data_size);
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
	*curr_check_sum = ntohl(*(uint32_t *) ( or_frame + len - 4));

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
	curr_frame->check_sum_match = (*curr_check_sum == ntohl(crc)) ? 1 : 0;
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


