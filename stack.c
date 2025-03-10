#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include "util.h"
#include "cs431vde.h"




int main(int argc, char *argv[]){
	/*
	char eth_addr[] = {x12,x9f,x41,x0d,x0e,x64}; //ethernet address
	int fds[2];

	uint8_t frame[1600];
	ssize_t frame_len;
	char *data_as_hex;

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
        printf("received frame, length %ld:\n", frame_len);
        puts(data_as_hex);
        free(data_as_hex);
    }

    if(frame_len < 0) {
        perror("read");
        exit(1);
    }
	*/

	uint8_t single_frame[] = {
					0x12,0x9f,0x41,0x0d,0x0e,0x64 //destination address
					0x12,0x9f,0x41,0x0d,0x0e,0x64 //source address
					0x12,0x9f,                    //type
					'P','a','y','l','o','a','d' //payload
					};

	struct frame_fields *frame = (struct frame_fields *)single_frame;
	uint8_t *mac_ptr = frame->dest_addr;
	for(int i=0; i<6; i++){
		printf("%02x", mac_ptr[i]);
	}
	printf("\n");
	return 0;
}


/*
void handle_frame(char *data_as_hex){
	//process frame
	struct frame_fields *s1 = (struct frame_fields *)data_as_hex;
	for(int i=0; i<48; i++){
		printf("%c", frame_fields->dest_addr);
	}
	printf("\n");
	//split frame into fields based on bytes order
	//retrieve ether net address from ether field
}
*/

