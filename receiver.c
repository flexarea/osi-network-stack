/*
 * receiver.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#include "util.h"
#include "cs431vde.h"

int
main(int argc, char *argv[])
{
    int fds[2];

    uint8_t frame[1600];
    ssize_t frame_len;
    char *data_as_hex;

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

    return 0;
}

