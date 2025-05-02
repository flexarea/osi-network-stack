#ifndef STACK_H
#define STACK_H

#include <sys/types.h>
#include <stdint.h>
#include <arpa/inet.h>
#include "icmp.h"
#include "ip.h"
#include "ethernet.h"

#define NUMBER_INTERFACES 3
#define CACHE_LENGTH 3
#define TABLE_LENGTH 3



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


void interface_receiver(struct frame_fields *frame_f, struct frame_flags *curr_frame, uint32_t *curr_check_sum, ssize_t *data_size, struct table_r *routing_table, struct arp_cache *arp_cache, struct interface *interface_list_, int net_id);

void handle_arp(struct frame_fields *frame_, uint8_t *or_frame, ssize_t len, struct packet_info *packet_inf, struct interface *interface_list_);

int is_interface(struct interface *interface_list, uint8_t *ip_addr);
void network_configuration(struct frame_fields *frame_f, struct frame_flags *curr_frame, uint32_t *curr_check_sum, ssize_t *data_size, struct table_r *routing_table, struct arp_cache *arp_cache, struct interface *interface_list_);
#endif
