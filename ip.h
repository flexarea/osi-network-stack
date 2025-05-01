#ifndef IP_H
#define IP_H

#include <sys/types.h>
#include <stdint.h>
#include <arpa/inet.h>

//stack.h struct declarations
struct frame_fields;
struct ip_header;
struct packet_info;
struct interface;
struct icmp;
struct arp_cache;
struct table_r;

typedef struct ip_header{
	uint8_t version_IHL; //version and IHL
	uint8_t type_of_service;
	uint16_t total_length;
	uint16_t identification;
	uint16_t flag_fragment_offset; //flag and fragment offset
	uint8_t ttl;
	uint8_t protocol;
	uint16_t header_checksum;
	uint8_t src_addr[4];
	uint8_t dest_addr[4];
}ip_header;

typedef struct packet_info{
	char src_ip_addr[INET_ADDRSTRLEN];
	char  dest_ip_addr[INET_ADDRSTRLEN];
	int valid_checksum;
	int valid_length;
}packet_info;

void handle_packet(ssize_t len, struct frame_fields *frame_f, uint8_t *or_frame, struct ip_header *packet, struct packet_info *packet_inf, struct icmp *curr_icmp, struct table_r *routing_table, struct arp_cache *arp_cache__, struct interface *interface_list_, int *receiver_id);

#endif 
