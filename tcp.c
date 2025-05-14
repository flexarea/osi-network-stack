#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "util.h"
#include "stack.h"
#include "cs431vde.h"
#include "crc32.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <poll.h>
#include <sys/random.h>
#include "tcp.h"
#include "icmp.h"
#include <inttypes.h>

//flag(1)FIN/ACK second transmission

int handle_tcp(ssize_t len, uint8_t *or_frame, struct ip_header *packet, struct packet_info *packet_inf,  struct tcp_connection *tcp_connection_table_, int flag){
	/*
	 * return 0(transmission) 1(2transmissions) -1(no transmissions)
	 * */
	printf("frame length: %d\n", (int) len);
	uint8_t tcp_ip_addr[] = TCP_IP; //interface listener IP
	struct tcp *tcp_header = (struct tcp *) (or_frame + 34); //extract tcp payload
	int connection_idx;
	int new_connection_status;

	uint8_t control_bits = ntohs(tcp_header->flag) & 0x3F;  //bitmask to extract flags only
	uint8_t data_offset = (ntohs(tcp_header->flag) >> 12) & 0xF; //extract data offset
	uint8_t tcp_header_len = data_offset*4;
	uint16_t data_octets = (uint16_t)len - 14 -  20 - tcp_header_len; //len - ethernet(14) - ip(20) - ethernet checksum (4)
	uint16_t tcp_segment_len = tcp_header_len + data_octets;

	printf("received data octets: %hu\n", data_octets);

	//verifiy ip addr match
	if(memcmp(tcp_ip_addr, packet->dest_addr, 4) != 0){
		printf("Dropping packet: unknown TCP IP address\n");
		return TCP_NONE;
	}

	//verify if port match
	if(ntohs(tcp_header->dest_port) != HOST_TCP_PORT){
		printf("Dropping packet: unknown port\n");
		return TCP_NONE;
	}

	//checksum
	uint16_t curr_checksum = calculate_tcp_checksum(packet, or_frame+34, tcp_segment_len);
	/*
	 * comment out for TCP testing*/

	 if(curr_checksum != 0){
	 printf("Dropping packet: Incorrect Checksum\n");
	 //return TCP_NONE;
	 }

	/*check if connection does not exist*/
	if((connection_idx = connection_lookup(tcp_connection_table_, packet->src_addr )) == -1){
		/*check if there is room for connection*/
		if((connection_idx = is_space(tcp_connection_table_)) == -1){
			printf("Cannot establish connection: Connection Limit Reached\n");
			return TCP_NONE;
		}	
	}

	//handle TCP payload
	new_connection_status = handle_tcp_payload(or_frame, tcp_header, packet_inf, control_bits, data_octets, data_offset, tcp_connection_table_, packet, connection_idx, flag, tcp_header_len);

	/*update ip field*/
	memcpy(packet->dest_addr, packet->src_addr, 4);
	memcpy(packet->src_addr, tcp_ip_addr, 4);

	//TTL
	packet->ttl = 64;

	//identification number
	packet->identification = htons(0); //should technically be random

	//initialize checksum to zero
	or_frame[50] = 0;
	or_frame[51] = 0;

	//compute TCP checksum
	uint16_t new_checksum = calculate_tcp_checksum(packet,  or_frame+34, tcp_segment_len);
	or_frame[50] = (new_checksum >>8) & 0xFF;
	or_frame[51] = new_checksum & 0xFF;

	//update ip string
	memcpy(packet_inf->dest_ip_addr, tcp_connection_table_[connection_idx].ip_str, INET_ADDRSTRLEN);
	return new_connection_status;
}

//create sequence number, modify ack number, send tcp packet
int handle_tcp_payload(uint8_t *or_frame, struct tcp *tcp_header_, struct packet_info *packet_inf, uint8_t control_bits, int8_t data_octets, int8_t data_offset_, struct tcp_connection *tcp_connection_table_, struct ip_header *packet, int connection_id, int flag, uint8_t tcp_header_len){

	uint32_t rcvd_ack;
	//check  flag
	switch(control_bits){
		//syn(2), ack(16), fin(1), syn/ack(18), ack/fin(10), psh/ack(24)
		case 2: 
			/*handle syn*/
			tcp_header_->ack_number = htonl((ntohl(tcp_header_->seq_number) + 1)); //create ack
			tcp_header_->seq_number = tcp_seq_generator(ntohl(tcp_header_->seq_number)); //generate seq number
			tcp_header_->flag = htons((data_offset_ << 12) | 0x12); //set syn/ack
			tcp_header_->dest_port = tcp_header_->src_port; //update port 
			tcp_header_->src_port = htons(HOST_TCP_PORT);

			/*add to connection table*/
			tcp_connection_table_[connection_id].connection_id = connection_id;
			memcpy(tcp_connection_table_[connection_id].host_ip_addr, packet->src_addr, 4);
			memcpy(tcp_connection_table_[connection_id].ip_str, packet_inf->src_ip_addr, INET_ADDRSTRLEN);

			/*record next_seq*/
			tcp_connection_table_[connection_id].next_seq = ntohl(tcp_header_->ack_number);

			//update external host's connection status
			tcp_connection_table_[connection_id].connection_status = 2;
			printf("run handle syn case\n");
			return TCP_REG;
		case 16:
			/*handle ack*/
			//check if host was in process to end connection

			rcvd_ack = tcp_header_->ack_number;
			tcp_header_->ack_number = htonl((ntohl(tcp_header_->seq_number) + data_octets)); //create ack
			tcp_header_->seq_number = rcvd_ack; //set seq_num

			/*record next_seq*/
			tcp_connection_table_[connection_id].next_seq = ntohl(tcp_header_->ack_number);
			tcp_connection_table_[connection_id].curr_seq = ntohl(tcp_header_->seq_number);

			if(tcp_connection_table_[connection_id].connection_status == 1){ //connection already exist
				if(data_octets > 0){
					//buffer/output data here
					uint8_t *data_ptr = or_frame + 34 + tcp_header_len;
					printf("received %d data octets\n", (int)data_octets);
					write(STDOUT_FILENO, data_ptr, data_octets);
					return TCP_REG;
				}

			}else{
				//update external host's connection status
				tcp_connection_table_[connection_id].connection_status = 1;
			}

			return TCP_NONE; //No transmission reply
		case 18:
			//handle SYN/ACK (currently not handled)
			break;
		case 10:
			//handle FIN/ACK
			if(flag){
				//second transmission
				tcp_header_->flag = htons((data_offset_ << 12) | 0x11); //set ACK
				return TCP_REG;	
			}

			rcvd_ack = tcp_header_->ack_number;
			tcp_header_->ack_number = htonl((ntohl(tcp_header_->seq_number) + data_octets)); //create ack
			tcp_header_->seq_number = rcvd_ack; //set seq_num

			//update TCP Payload field
			tcp_header_->flag = htons((data_offset_ << 12) | 0x10); //set ACK
			tcp_header_->dest_port = tcp_header_->src_port; //update port
			tcp_header_->src_port = htons(HOST_TCP_PORT);

			/*record next_seq*/
			tcp_connection_table_[connection_id].next_seq = ntohl(tcp_header_->ack_number);
			tcp_connection_table_[connection_id].curr_seq = ntohl(tcp_header_->seq_number);

			return TCP_FIN_ACK;
		default:
			printf("unsupported flag");
			return -1;
	}

	return -1;
}

//calculate data length

//handle action connection
int connection_lookup(struct tcp_connection *tcp_connection_table_, uint8_t *ip_addr){

	for(int i=0; i<TCP_CONNECTION_LIMIT; i++){
		if(memcmp(tcp_connection_table_[i].host_ip_addr, ip_addr, 4) == 0){
			return i;
		}
	}
	return -1;
}

//check connection table 
int is_space(struct tcp_connection *tcp_connection_table_){
	for(int i=0; i<TCP_CONNECTION_LIMIT; i++){
		if(!tcp_connection_table_[i].connection_status){
			return i;
		}
	}
	return -1;
}

uint32_t tcp_seq_generator(uint32_t prev){
	uint32_t buf;	

	do{
		if(getrandom(&buf, sizeof(buf), 0) == -1){
			perror("getrandom");
			return 0;
		}
	}while(buf == prev);

	return buf;
}

uint16_t calculate_tcp_checksum(struct ip_header *packet, uint8_t *tcp_segment, uint16_t tcp_len) {
	//Build pseudo-header (12 bytes)
    uint8_t pseudo_header[12];
    memcpy(pseudo_header, packet->src_addr, 4);      // Source IP
    memcpy(pseudo_header + 4, packet->dest_addr, 4); // Dest IP
    pseudo_header[8] = 0;                        // Zero
    pseudo_header[9] = 6;              // Protocol=6

	uint16_t tcp_len_net = htons(tcp_len);
	memcpy(pseudo_header + 10, &tcp_len_net, 2); //length

    // Zero TCP checksum field (offset 16)
    uint8_t *tcp_copy = malloc(tcp_len);
    memcpy(tcp_copy, tcp_segment, tcp_len);
    memset(tcp_copy + 16, 0, 2); // Clear checksum field

    // Combine pseudo-header and TCP segment
    uint8_t *buffer = malloc(12 + tcp_len);
    memcpy(buffer, pseudo_header, 12);
    memcpy(buffer + 12, tcp_copy, tcp_len);

    // Compute checksum
    uint16_t checksum = ip_checksum(buffer, 12 + tcp_len);

	// clean up
    free(tcp_copy);
    free(buffer);

    return checksum;

}

