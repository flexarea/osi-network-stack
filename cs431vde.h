/*
 * cs431vde.h
 */

#include <stdlib.h>
#include <stdint.h>

int connect_to_vde_switch(int fds[2], char *cmd[]);
ssize_t receive_ethernet_frame(int fd, void *buf);
void send_ethernet_frame(int fd, void *frame, uint16_t len);

