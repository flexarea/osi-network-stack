#ifndef ICMP_H
#define ICMP_H

#include <sys/types.h>
#include <stdint.h>
#include <arpa/inet.h>

// stack.h struct declarations
struct frame_fields;
struct ip_header;
struct packet_info;
struct interface;

typedef struct icmp{
	int type;
	int code;
}icmp;

void handle_icmp(ssize_t len, struct frame_fields *frame_f, uint8_t *or_frame, struct ip_header *packet, struct packet_info *packet_inf, struct icmp *curr_icmp, struct interface *interface_list_, int transmitter_id);

uint16_t rand_generator();

#endif
