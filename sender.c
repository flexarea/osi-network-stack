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

	//0xff,0xff,0xff,0xff,0xff,0xff //broadcasating
	//0x12,0x9f,0x41,0x0d,0x0e,0x64, //destination address

	uint8_t frame[1600] = {
		0x12,0x9f,0x41,0x0d,0x0e,0x63, //destination address
		//0xff,0xff,0xff,0xff,0xff,0xff, //broadcasating
		0x12,0x9f,0x41,0x0d,0x0e,0x64, //source address
		0x12,0x9f,                    //type
		'Y','Z','A','B','C','D','E','F',
		'Y','Z','A','B','C','D','E','F',
		'Y','Z','A','B','C','D','E','F',
		'G','H','I','J','K','L','M','N',
		'Y','Z','A','B','C','D','E','F',
		'O','P','Q','R','S','T','U','V',
	};


	int connect_to_remote_switch = 0;
	char *local_vde_cmd[] = { "vde_plug", NULL };
	char *remote_vde_cmd[] = { "ssh", "pjohnson@weathertop.cs.middlebury.edu",
		"/home/pjohnson/cs431/bin/vde_plug",
		NULL };
	char **vde_cmd = connect_to_remote_switch ? remote_vde_cmd : local_vde_cmd;

	if(connect_to_vde_switch(fds, vde_cmd) < 0) {
		printf("Could not connect to switch, exiting.\n");
		exit(1);
	}

	//memset(frame, '\xff', 64);
	//compute cfs
	uint32_t crc = crc32(0, frame, 62);	//3rd param: dst_addr+src_addr+type+payload
	uint8_t *cfs_ptr = (uint8_t *)frame + 62;
	memcpy(cfs_ptr, &crc, 4);
	frame_len = 66;

	printf("sending frame, length %ld\n", frame_len);
	send_ethernet_frame(fds[1], frame, frame_len);

	/* If the program exits immediately after sending its frames, there is a
	 * possibility the frames won't actually be delivered.  If, for example,
	 * the "remote_vde_cmd" above is used, the user might not even finish
	 * typing their password (which is accepted by a child process) before
	 * this process terminates, which would result in send frames not actually
	 * arriving.  Therefore, we pause and let the user manually end this
	 * process. */

	printf("Press Control-C to terminate sender.\n");
	pause();

	return 0;
}
