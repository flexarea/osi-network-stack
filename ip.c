
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "util.h"
#include "stack.h"
#include "icmp.h"
#include "ip.h"
#include "cs431vde.h"
#include "crc32.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <poll.h>


//function to extract and process ip header fields
void handle_packet(ssize_t len, struct frame_fields *frame_f, uint8_t *or_frame, struct ip_header *packet, struct packet_info *packet_inf, struct icmp *curr_icmp, struct table_r *routing_table, struct arp_cache *arp_cache__, struct interface *interface_list_, int *receiver_id){
	//receiver_id is the index of the interface that received the packet
	//printf("handling ip packet...\n");


	struct in_addr ip_addr;
	int lg_pfx_idx = -1; //longest subnet prefix index
	int lg_prefix = -1;
	//int arp_idx; 
	int error = 0;
	int transmitter_idx = *receiver_id;
	uint8_t *final_dest_addr;

	//convert dest address to ip str
	inet_ntop(AF_INET, &(packet->dest_addr), packet_inf->dest_ip_addr, INET_ADDRSTRLEN);	
	//convert src address to ip str
	inet_ntop(AF_INET, &(packet->src_addr), packet_inf->src_ip_addr, INET_ADDRSTRLEN);	
	//convert dest ip to bin
	if(!inet_aton(packet_inf->dest_ip_addr, &ip_addr)){
		printf("error converting ip to binary\n");
	}

	packet_inf->valid_length = (ntohs(packet->total_length) > 20) ? 1 : 0;
	packet_inf->valid_checksum = (ip_checksum(or_frame + 14, 20) == 0) ? 1 : 0;

	if(!packet_inf->valid_checksum) return;
	if(!packet_inf->valid_length) return;


	if(packet->ttl == 1){
		curr_icmp->type = 11;
		curr_icmp->code = 0;
		error = 1;
		//return;
	}else if(packet->ttl < 1){
		curr_icmp->type = 11;
		curr_icmp->code = 0;
		error = 1;
		printf("dropping packet from %s to %s (TTL exceeded)\n", packet_inf->src_ip_addr, packet_inf->dest_ip_addr);
		return;
	}

	/*destination ip matches local interface*/
	if(is_interface(interface_list_, packet->dest_addr) >= 0){
			//dest_addr = interface_list_[interface_idx].mac_addr;
			printf("digesting packet\n");
			return;
		}

	if(!error){
			for(int i=0; i<TABLE_LENGTH; i++){
				char genmask_str[INET_ADDRSTRLEN]; 
				char dest_str[INET_ADDRSTRLEN];
				struct in_addr dest_addr, genmask_addr, result;
				int curr_lg_prefix;

				//convert addresses to ip
				inet_ntop(AF_INET, routing_table[i].genmask, genmask_str, INET_ADDRSTRLEN);
				inet_ntop(AF_INET, routing_table[i].dest, dest_str, INET_ADDRSTRLEN);

				//convert to bin
				inet_aton(genmask_str, &genmask_addr);
				inet_aton(dest_str, &dest_addr);
				//bitwise-and
				result.s_addr = ip_addr.s_addr & genmask_addr.s_addr;

				//check matching dest addr
				if(result.s_addr == dest_addr.s_addr){
					curr_lg_prefix = __builtin_popcount(ntohl(genmask_addr.s_addr));
					if(curr_lg_prefix > lg_prefix){
						lg_prefix = curr_lg_prefix;
						lg_pfx_idx = i;
					}	
				}
			}

			char *real_path;

			if(lg_pfx_idx != -1){
				//save the interface id that matches the dest addr found in routing table
				transmitter_idx = routing_table[lg_pfx_idx].interface_id;
				//check if packet is destined to local network
				if (memcmp(routing_table[lg_pfx_idx].gateway, "\x00\x00\x00\x00", 4) == 0 ){
					real_path = (char *)packet->dest_addr;
				}else{
					real_path = (char *)routing_table[lg_pfx_idx].gateway;
				}
				// arp lookup
				int found_mac = 0;

				for(int i=0; i<CACHE_LENGTH; i++){
					if(memcmp(arp_cache__[i].ip_addr, real_path, 4) == 0){
						//arp_idx = i;
						final_dest_addr = arp_cache__[i].mac_addr;
						found_mac = 1;
					}	
				}

				//check for network errors
				if(!found_mac){
					//host unreachable
					curr_icmp->type = 3;
					curr_icmp->code = 1;
					error = 1;
				}
			}else{
				//network unreacheable
				curr_icmp->type = 3;
				curr_icmp->code = 0;
				error = 1;
			}
			if(!error) packet->ttl--;
	}

	//check for error again
	if(error){
		transmitter_idx = *receiver_id; //use the interface that received the packet
	}	


	//encapsulation here
	encapsulation(frame_f, packet, len, or_frame, final_dest_addr, curr_icmp, interface_list_, error, packet_inf, transmitter_idx);
	send_ethernet_frame(interface_list_[transmitter_idx].switch_[1], or_frame, len);
	printf("forwading packet to %s\n", packet_inf->dest_ip_addr);
}
