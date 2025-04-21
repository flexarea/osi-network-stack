/*
 * sender.c
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

#include "util.h"
#include "cs431vde.h"
#include "crc32.h"

	int
main(int argc, char *argv[])
{
	int fds[2];
	ssize_t frame_len;
	ssize_t rcv_frame_len;
	uint8_t rcv_frame[1600];

	char *data_as_hex;

	// ARP request encapsulated in ethernet frame (60 bytes)
	uint8_t frame[1600] = {
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // Destination (broadcast)
		0x12, 0x9f, 0x41, 0x0d, 0x0e, 0x64, // Source MAC
		0x08, 0x06,                         // Type (ARP = 0x0806)

		// ARP Packet (28 bytes)
		0x00, 0x01,                         // Hardware type (Ethernet = 1)
		0x08, 0x00,                         // Protocol type (IPv4 = 0x0800)
		0x06,                               // Hardware address length (MAC = 6 bytes)
		0x04,                               // Protocol address length (IPv4 = 4 bytes)
		0x00, 0x01,                         // opcode (request = 1)
		0x12, 0x9f, 0x41, 0x0d, 0x0e, 0x64, //Sender hardware address
		0x01, 0x02, 0x02, 0x05,				//Sender protocol address
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //Target hardware
		0x01, 0x02, 0x01, 0x01,            //Target protocol address

		//padding here (18 bytes)
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
			//FCS
	};


	int connect_to_remote_switch = 0;
	char *local_vde_cmd[] = {"vde_plug", "/home/entuyenabo/cs432/cs431/tmp/net1.vde", NULL};
	char *remote_vde_cmd[] = { "ssh", "pjohnson@weathertop.cs.middlebury.edu","/home/pjohnson/cs431/bin/vde_plug",NULL };
	char **vde_cmd = connect_to_remote_switch ? remote_vde_cmd : local_vde_cmd;

	if(connect_to_vde_switch(fds, vde_cmd) < 0) {
		printf("Could not connect to switch, exiting.\n");
		exit(1);
	}

	//memset(frame, '\xff', 64);

	//compute ip checksum	
	uint16_t ip_checksum_ = ip_checksum(frame+14);

	frame[24] = (ip_checksum_ >> 8) & 0xFF;
	frame[25] = ip_checksum_ & 0xFF;

	//compute cfs
	uint32_t crc = crc32(0, frame, 60);	//3rd param: dst_addr+src_addr+type+payload

	uint8_t *cfs_ptr = (uint8_t *)frame + 60; //
	memcpy(cfs_ptr, &crc, 4);
	frame_len = 64;

	printf("sending frame, length %ld\n", frame_len);
	send_ethernet_frame(fds[1], frame, frame_len);

	/* If the program exits immediately after sending its frames, there is a
	 * possibility the frames won't actually be delivered.  If, for example,
	 * the "remote_vde_cmd" above is used, the user might not even finish
	 * typing their password (which is accepted by a child process) before
	 * this process terminates, which would result in send frames not actually
	 * arriving.  Therefore, we pause and let the user manually end this
	 * process. 
	 * */

	printf("Press Control-C to terminate sender.\n");
	pause();

	while((rcv_frame_len = receive_ethernet_frame(fds[0], rcv_frame)) > 0) {
		data_as_hex = binary_to_hex(rcv_frame, rcv_frame_len);
		printf("received frame, length %ld:\n", rcv_frame_len);
		puts(data_as_hex);
		free(data_as_hex);
	}

	if(rcv_frame_len < 0) {
		perror("read");
		exit(1);
	}
	return 0;
}
