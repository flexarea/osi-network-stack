#ifndef STACK_H
#define STACK_H

#include <sys/types.h>
#include <stdint.h>
#include <arpa/inet.h>

#define NUMBER_INTERFACES 3

typedef struct frame_fields{
	uint8_t dest_addr[6];
	uint8_t src_addr[6];
	uint16_t type;
}frame_fields;


typedef struct frame_flags{
	int is_broadcast;	
	int is_for_me;	
	int check_sum_match;	
	int valid_length ;	
	uint32_t rcv_check_sum;
}frame_flags;

typedef struct router{
	uint8_t interface_mac;
	uint8_t interface_ip;
}router;


typedef struct interface{
	/*
	the quantity, IP addresses, MAC addresses, and network parameters for each simulated interface;
	*/
	uint8_t mac_addr[6];
	uint8_t ip_addr[4];
	int switch_[2];
}interface;

//routing table row
typedef struct table_r{
	uint8_t dest[4];
	uint8_t gateway[4];
	uint8_t genmask[4];
	uint8_t flag[4];
	uint8_t Netif[3];
	uint8_t subnet_mask;
	int interface_id;
	//add more below
}table_r;

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

typedef struct arp{
	uint16_t hardware_type;
	uint16_t protocol_type;
	uint8_t hardware_size;
	uint8_t protocol_size;
	uint16_t opcode; //1 for request 2 for reply
	uint8_t sender_mac[6];
	uint8_t sender_ip[4];
	uint8_t target_mac[6];
	uint8_t target_ip[6];
}arp;

typedef struct arp_cache{
	uint8_t ip_addr[4];
	uint8_t mac_addr[6];
}arp_cache;

typedef struct icmp{
	int type;
	int code;
}icmp;

void handle_frame(char *data_as_hex, ssize_t len, struct frame_fields *frame_f, struct frame_flags *curr_frame, ssize_t *data_size, uint32_t *curr_check_sum, const uint8_t *mac_addr, uint8_t *or_frame);
void interface_receiver(struct frame_fields *frame_f, struct frame_flags *curr_frame, uint32_t *curr_check_sum, ssize_t *data_size, const uint8_t *mac_addr, struct table_r *routing_table, struct arp_cache *arp_cache, struct interface *interface_list_);
void encapsulation(struct frame_fields *frame_, struct ip_header *packet_, ssize_t len, uint8_t *or_frame, uint8_t *dest_addr, struct icmp *curr_icmp, struct interface *interface_list_, int error_, struct packet_info *packet_inf, int transmitter_id);
void handle_packet(ssize_t len, struct frame_fields *frame_f, uint8_t *or_frame, struct ip_header *packet, struct packet_info *packet_inf, struct icmp *curr_icmp, struct table_r *routing_table, struct arp_cache *arp_cache__, struct interface *interface_list_, int receiver_id);
void handle_arp(struct frame_fields *frame_, uint8_t *or_frame, ssize_t len, struct packet_info *packet_inf, struct interface *interface_list_);
void handle_icmp(ssize_t len, struct frame_fields *frame_f, uint8_t *or_frame, struct ip_header *packet, struct packet_info *packet_inf, struct icmp *curr_icmp, struct interface *interface_list_, int transmitter_id);
int is_interface(struct interface *interface_list, uint8_t *ip_addr);
#endif
