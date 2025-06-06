#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "util.h"
#include "stack.h"
#include "icmp.h"
#include "tcp.h"
#include "ethernet.h"
#include "cs431vde.h"
#include "crc32.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <poll.h>

//const int TABLE_LENGTH = 3;
//const int CACHE_LENGTH = 3;
//const int NUMBER_INTERFACES = 3;

/*interface ethernet addr*/
const uint8_t eth_addr[] = {0x12,0x9f,0x41,0x0d,0x0e,0x64}; //tap0
const uint8_t eth_addr1[] = {0x12,0x9f,0x41,0x0d,0x0e,0x65}; //tap1
const uint8_t eth_addr2[] = {0x12,0x9f,0x41,0x0d,0x0e,0x66}; //tap2
/*interface ip addr*/
const uint8_t interface_ip[]={0xC0,0xA8,0x00,0x05};     //Interface1 IP address
const uint8_t interface2_ip[]={0x01,0x03,0x01,0x01};       //Interface2 IP address
const uint8_t interface3_ip[]={0x01,0x04,0x01,0x01};       //Interface3 IP address


struct frame_flags cf_flag; //current frame flag
struct frame_fields frame_header;
ssize_t d_size;
uint32_t cur_cfs; //received cfs
 
int main(int argc, char *argv[]){

	struct table_r *interface_routing_table = (struct table_r *) malloc(TABLE_LENGTH*sizeof(struct table_r));
	struct arp_cache *interface_arp_cache = (struct arp_cache *) malloc(CACHE_LENGTH*sizeof(struct arp_cache));
	struct tcp_connection *tcp_connection_table = (struct tcp_connection *)malloc(TCP_CONNECTION_LIMIT*sizeof(struct tcp_connection));

	/*interface routing routing_table configuration*/
	memcpy(interface_routing_table[0].dest, "\x01\x02\x00\x00",4);
	memcpy(interface_routing_table[0].gateway, "\x00\x00\x00\x00",4); //subnework 1 (1.2/16)
	memcpy(interface_routing_table[0].genmask, "\xff\xff\x00\x00",4);
	memcpy(interface_routing_table[1].dest, "\x01\x03\x00\x00",4);
	memcpy(interface_routing_table[1].gateway, "\x00\x00\x00\x00",4); //subnetwork 2 (1.3/16)
	memcpy(interface_routing_table[1].genmask, "\xff\xff\x00\x00",4);
	memcpy(interface_routing_table[2].dest, "\x01\x04\x00\x00",4);
	memcpy(interface_routing_table[2].gateway, "\x00\x00\x00\x00",4); //subnetwork 3 (1.4/16)
	memcpy(interface_routing_table[2].genmask, "\xff\xff\x00\x00",4);
	/*assign ids to interfaces in routing table*/
	interface_routing_table[0].interface_id = 0;
	interface_routing_table[1].interface_id = 1;
	interface_routing_table[2].interface_id = 2;

	/*ARP cache configuration*/
	memcpy(interface_arp_cache[0].ip_addr, "\x01\x02\x01\x03",4);
	memcpy(interface_arp_cache[0].mac_addr, "\x12\x9f\x41\x0d\x0e\x67",6); //send to host (next hop)
	memcpy(interface_arp_cache[1].ip_addr, "\x01\x03\x01\x02",4);
	memcpy(interface_arp_cache[1].mac_addr, "\x12\x9f\x41\x0d\x0e\x68",6); //send to host (next hop)

	/*Interface configuration*/
	struct interface *interface_list = (struct interface *) malloc(NUMBER_INTERFACES * sizeof(struct interface));
	if(interface_list == NULL){
		perror("interface list failed to malloc");
	}
	memcpy(interface_list[0].mac_addr, eth_addr ,6);
	memcpy(interface_list[1].mac_addr, eth_addr1 ,6);
	memcpy(interface_list[2].mac_addr, eth_addr2 ,6);
	memcpy(interface_list[0].ip_addr, interface_ip ,4);
	memcpy(interface_list[1].ip_addr, interface2_ip ,4);
	memcpy(interface_list[2].ip_addr, interface3_ip ,4);

	/*Configure TCP connection table*/
	for(int i=0; i<TCP_CONNECTION_LIMIT; i++){
		memset(&tcp_connection_table[i], 0, sizeof(struct tcp_connection));
	}


	network_configuration(&frame_header, &cf_flag, &cur_cfs, &d_size, interface_routing_table, interface_arp_cache, interface_list, tcp_connection_table);
	free(interface_routing_table);
	free(interface_arp_cache);
	free(interface_list);
	free(tcp_connection_table);
	return 0;
}

void network_configuration(struct frame_fields *frame_f, struct frame_flags *curr_frame, uint32_t *curr_check_sum, ssize_t *data_size, struct table_r *routing_table, struct arp_cache *arp_cache, struct interface *interface_list_, struct tcp_connection *tcp_connection_table_){

	int connect_to_remote_switch = 0;
	char *local_vde_cmd[] = {"vde_plug", "/tmp/net0.vde", NULL}; //replace /tmp/net1.vde with /tmp/net0.vde for TCP
	char *local_vde_cmd2[] = {"vde_plug", "/home/entuyenabo/cs432/cs431/tmp/net2.vde", NULL};
	char *local_vde_cmd3[] = {"vde_plug", "/home/entuyenabo/cs432/cs431/tmp/net3.vde", NULL};
	char *remote_vde_cmd[] = {"ssh", "entuyenabo@weathertop.cs.middlebury.edu", "/home/entuyenabo/cs431/bin/vde_plug", NULL};

	char **vde_cmd = connect_to_remote_switch ? remote_vde_cmd : local_vde_cmd;
	char **vde_cmd2 = connect_to_remote_switch ? remote_vde_cmd : local_vde_cmd2;
	char **vde_cmd3 = connect_to_remote_switch ? remote_vde_cmd : local_vde_cmd3;

	//connect to switch1
	if(connect_to_vde_switch(interface_list_[0].switch_, vde_cmd) < 0){
		printf("Could not connect to switch, exiting.\n");
		exit(1);
	}
	//connect to switch2
	if(connect_to_vde_switch(interface_list_[1].switch_, vde_cmd2) < 0){
		printf("Could not connect to switch, exiting.\n");
		exit(1);
	}

	//connect to switch3
	if(connect_to_vde_switch(interface_list_[2].switch_, vde_cmd3) < 0){
		printf("Could not connect to switch, exiting.\n");
		exit(1);
	}
	
	/*polling implementation*/
	//add 1 for STDIN
	struct pollfd *pfds =  malloc(NUMBER_INTERFACES+1 * sizeof(struct pollfd));
	if (pfds == NULL) {
        perror("Failed to allocate memory for poll fds");
        exit(1);
    }

	//assign POLLIN to each pollfd events
	for(int i=0; i<NUMBER_INTERFACES; i++){
		pfds[i].fd = interface_list_[i].switch_[0];
		pfds[i].events = POLLIN;
	}

	//configure stdin polling
	pfds[NUMBER_INTERFACES].fd = STDIN_FILENO; //user input
	pfds[NUMBER_INTERFACES].events = POLLIN;

	uint8_t tcp_mac[] = TCP_MAC;
    uint8_t tcp_ip[] = TCP_IP;

	int default_tcp_interface_id = 0;

	while(1){
		if(poll(pfds, NUMBER_INTERFACES + 1, -1) == -1){
			printf("poll");
			exit(1);
		}
	
		for(int i=0; i<NUMBER_INTERFACES; i++){
			if(pfds[i].revents & POLLIN){

				interface_receiver(frame_f, curr_frame, curr_check_sum, data_size, routing_table, arp_cache, interface_list_, i, tcp_connection_table_);
			}
		}

		if(pfds[NUMBER_INTERFACES].revents & POLLIN){
			interaction_handler(default_tcp_interface_id, tcp_connection_table_, tcp_mac, tcp_ip, interface_list_);
		}
	}
	
	free(pfds);
}

void interface_receiver(struct frame_fields *frame_f, struct frame_flags *curr_frame, uint32_t *curr_check_sum, ssize_t *data_size, struct table_r *routing_table, struct arp_cache *arp_cache, struct interface *interface_list_, int net_id, struct tcp_connection *tcp_connection_table_){

	int receiver = net_id;
	char *data_as_hex;
	uint8_t frame[1600];
	ssize_t frame_len;

	if ((frame_len = receive_ethernet_frame(interface_list_[receiver].switch_[0], frame)) > 0) {
		data_as_hex = binary_to_hex(frame, frame_len);

		//retrieve frame fields information
		frame_f = (struct frame_fields *)frame;
		handle_frame(data_as_hex, frame_len, frame_f,  curr_frame, data_size,  curr_check_sum, frame, interface_list_, &receiver);
		//jump to beginning of ip header
		uint8_t *ether_payload = frame + 14;
		//cast to ip struct (extract ip header fields)
		struct ip_header *ip_packet = (struct ip_header *) ether_payload;
		struct icmp curr_packet_icmp;
		curr_packet_icmp.type = 1;
		struct packet_info ip_packet_info;


		if(curr_frame->valid_length == 0){
			printf("ignoring %d-byte frame (short)\n", (int)frame_len);
			goto skip;
		}

		if(curr_frame->check_sum_match == 0){
		//	printf("ignoring %d-byte frame", (int)frame_len);
		//	printf("(bad fcs: got 0x%08x, expected 0x%08x)\n", curr_frame->rcv_check_sum, *curr_check_sum);
			//printf("WARNING: Bad Ethernet FCS but continuing anyway for testing\n");
			//goto skip;
		}

		if(curr_frame->is_broadcast){
			if(ntohs(frame_f->type) == 2054){
				handle_arp(frame_f,frame, frame_len, &ip_packet_info, interface_list_);
				goto skip;
			}
			printf("received %d-byte broadcast frame from %02x %02x %02x %02x %02x %02x\n", (int)frame_len, 
					frame_f->src_addr[0],
					frame_f->src_addr[1],
					frame_f->src_addr[2],
					frame_f->src_addr[3],
					frame_f->src_addr[4],
					frame_f->src_addr[5]);
			goto skip;
		}

		if(curr_frame->is_for_me){
			printf("received %d-byte frame for me from %02x %02x %02x %02x %02x %02x\n", (int)frame_len, 
					frame_f->src_addr[0],
					frame_f->src_addr[1],
					frame_f->src_addr[2],
					frame_f->src_addr[3],
					frame_f->src_addr[4],
					frame_f->src_addr[5]);
			//continue;
			//
			if(ntohs(frame_f->type) == 2048){
				handle_packet(frame_len, frame_f, frame, ip_packet, &ip_packet_info, &curr_packet_icmp, routing_table, arp_cache, interface_list_, &receiver, tcp_connection_table_);
				//check ip icmp
				if(!ip_packet_info.valid_length){ //actually change comparison here, it's wrong
					printf("somewhere here\n");
					printf("dropping packet from %s (wrong length)\n", ip_packet_info.src_ip_addr);
					goto skip;
				}

				if(!ip_packet_info.valid_checksum){
					//printf("inside checksum check\n");
					//printf("dropping packet from %s (bad IP header checksum)\n", ip_packet_info.src_ip_addr);
					//goto skip;
					//printf("WARNING: Bad Ethernet FCS but continuing anyway for testing\n");

				}	
			}
			goto skip;
		}else{
			printf("ignoring %d-byte frame (not for me)\n", (int)frame_len);
			goto skip;
		}

skip:
		free(data_as_hex);
	}

	if(frame_len < 0) {
		perror("read");
		exit(1);
	}
}
