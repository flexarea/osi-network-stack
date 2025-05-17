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

	//skip all these check in case of retransmission
	printf("frame length: %d\n", (int) len);
	uint8_t tcp_ip_addr[] = TCP_IP; //interface listener IP
	uint8_t tmp_addr[4];
	uint16_t tmp_port;
	struct tcp *tcp_header = (struct tcp *) (or_frame + 34); //extract tcp payload
	int connection_idx;
	int new_connection_status;

	/*extract tcp header flags*/
	uint8_t control_bits = (!flag) ? ntohs(tcp_header->flag) & 0x3F : 17;  //bitmask to extract flags only
	uint8_t data_offset = (ntohs(tcp_header->flag) >> 12) & 0xF; //extract data offset
	uint8_t tcp_header_len = data_offset*4;
	uint16_t data_octets = (uint16_t)len - 14 -  20 - tcp_header_len;
	uint16_t tcp_segment_len = tcp_header_len + data_octets;
	uint16_t curr_checksum;

	/*save ip and port to find interface in FIN/ACK case */
	memcpy(tmp_addr, packet->dest_addr, 4);
	tmp_port = ntohs(tcp_header-> dest_port);

	if(!flag){

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
		curr_checksum = calculate_tcp_checksum(packet, or_frame+34, tcp_segment_len, 1);
		if(curr_checksum != 0){
			printf("Dropping packet: Incorrect Checksum\n");
			//return TCP_NONE;
		}

		/*case interface addr is src addr*/
		memcpy(tmp_addr, packet->src_addr, 4); 
		tmp_port = ntohs(tcp_header-> src_port);
	}

	/*check if connection does not exist*/
	if((connection_idx = connection_lookup(tcp_connection_table_, tmp_addr, tmp_port)) == -1){
		/*check if there is room for connection*/
		if((connection_idx = is_space(tcp_connection_table_)) == -1){
			printf("Cannot establish connection: Connection Limit Reached\n");
			return TCP_NONE;
		}	
		printf("found space for connection\n");
		printf("New connection ID %d\n", connection_idx);
	}

	//handle TCP payload
	new_connection_status = handle_tcp_payload(or_frame, tcp_header, packet_inf, control_bits, data_octets, data_offset, tcp_connection_table_, packet, connection_idx, flag, tcp_header_len);

	/*update ip field (unless it's FIN/ACK retransmission)*/
	if(!flag){

		memcpy(packet->dest_addr, packet->src_addr, 4);
		memcpy(packet->src_addr, tcp_ip_addr, 4);

		//TTL
		packet->ttl = 64;

		//update ip string
		memcpy(packet_inf->dest_ip_addr, tcp_connection_table_[connection_idx].ip_str, INET_ADDRSTRLEN);
	}

	if(new_connection_status > 1){
		tcp_segment_len -= data_octets;		
	}

	//identification number
	packet->identification = (flag) ? htons(1) : htons(0); //should technically be random

	//initialize checksum to zero
	or_frame[50] = 0;
	or_frame[51] = 0;

	//compute TCP checksum (always)
	uint16_t new_checksum = calculate_tcp_checksum(packet,  or_frame+34, tcp_segment_len, 0);
	tcp_header->checksum = new_checksum;

	//print current connection status
	printf("\n");
	printf("TCP TRANSMISSION INFO\n");
	printf("Current connection status\n");
	printf("Next Sequence: %u\n", tcp_connection_table_[connection_idx].next_seq);	
	printf("Next connection ACK: %u\n", tcp_connection_table_[connection_idx].curr_seq);	
	printf("\n");

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
			//tcp_connection_table_[connection_id].connection_id = connection_id;
			memcpy(tcp_connection_table_[connection_id].host_ip_addr, packet->src_addr, 4);
			memcpy(tcp_connection_table_[connection_id].ip_str, packet_inf->src_ip_addr, INET_ADDRSTRLEN);
			tcp_connection_table_[connection_id].port = ntohs(tcp_header_->dest_port);

			/*record next_seq*/
			tcp_connection_table_[connection_id].next_seq = ntohl(tcp_header_->ack_number);

			//update external host's connection status
			tcp_connection_table_[connection_id].connection_status = 2;
			printf("run handle syn case\n");
			printf("connection status: %d\n", tcp_connection_table_[connection_id].connection_status); //connection already exist
			return TCP_REG;

		case 16:
			/*handle ack*/
			//check if host was in process to end connection

			if(tcp_connection_table_[connection_id].connection_status == 0){ //connection already exist
				printf("connection status: %d\n", tcp_connection_table_[connection_id].connection_status); //connection already exist
				printf("Digest packet (Connection Closed)\n");
				return TCP_NONE;
			}


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
				//check if it's last ACK or SYN/ACK response
				if(tcp_connection_table_[connection_id].connection_status == 2){
					//update external host's connection status
					tcp_connection_table_[connection_id].connection_status = 1;
					printf("connection status: %d\n", tcp_connection_table_[connection_id].connection_status); //connection already exist
					printf("Digesting ACK (No Reply)\n");
				}
			}

			return TCP_NONE; //No transmission reply
		case 24:
			/*handle PSH/ACK */

			rcvd_ack = tcp_header_->ack_number;
			tcp_header_->ack_number = htonl((ntohl(tcp_header_->seq_number) + data_octets)); //create ack
			tcp_header_->seq_number = rcvd_ack; //set seq_num


			/*record next_seq*/
			tcp_connection_table_[connection_id].next_seq = ntohl(tcp_header_->ack_number);
			tcp_connection_table_[connection_id].curr_seq = ntohl(tcp_header_->seq_number);

			//update TCP Payload field
			tcp_header_->flag = htons((data_offset_ << 12) | 0x10); //set ACK
			tcp_header_->dest_port = tcp_header_->src_port; //update port
			tcp_header_->src_port = htons(HOST_TCP_PORT);

			//output received data
			uint8_t *data_ptr = or_frame + 34 + tcp_header_len;
			printf("\n");
			printf("received %d data octets\n", (int)data_octets);
			printf("\n");
			write(STDOUT_FILENO, data_ptr, data_octets);

			//resize packet
			uint16_t new_ip_len = ntohs(packet->total_length) - data_octets;
			packet->total_length = htons(new_ip_len);
			//tcp_header->flag = htons((ntohs(tcp_header->flag) & 0x0FFF) | (8 << 12));


			return data_octets; // PSH/ACK case
		case 18:
			//handle SYN/ACK (currently not handled)
			break;
		case 17:
			//handle FIN/ACK
			printf("Handling FIN/ACK\n");

			if(flag){
				//second transmission
				printf("Executing second transmission\n");
				/*
				   uint32_t curr_seq = tcp_connection_table_[connection_id].curr_seq;
				   uint32_t next_seq = tcp_connection_table_[connection_id].next_seq;

				   tcp_header_->seq_number = htonl(curr_seq);
				   tcp_header_->ack_number = htonl(next_seq);
				   */


				tcp_header_->flag = htons((data_offset_ << 12) | 0x11); //set FIN/ACK
				tcp_connection_table_[connection_id].connection_status = 0; //close connection
				printf("connection status: %d\n", tcp_connection_table_[connection_id].connection_status); //connection already exist
				return TCP_REG;	
			}

			rcvd_ack = tcp_header_->ack_number;
			tcp_header_->ack_number = htonl((ntohl(tcp_header_->seq_number) + 1)); //create ack
			tcp_header_->seq_number = rcvd_ack; //set seq_num

			//update TCP Payload field
			tcp_header_->flag = htons((data_offset_ << 12) | 0x10); //set ACK
			tcp_header_->dest_port = tcp_header_->src_port; //update port
			tcp_header_->src_port = htons(HOST_TCP_PORT);

			/*record next_seq*/
			tcp_connection_table_[connection_id].next_seq = ntohl(tcp_header_->ack_number);
			tcp_connection_table_[connection_id].curr_seq = ntohl(tcp_header_->seq_number);

			printf("connection status: %d\n", tcp_connection_table_[connection_id].connection_status); //connection already exist
			return TCP_FIN_ACK;
		default:
			printf("unsupported flag: (%d)\n", control_bits);
			return TCP_NONE;
	}

	return -1;
}

//calculate data length

//handle action connection
int connection_lookup(struct tcp_connection *tcp_connection_table_, uint8_t *ip_addr, uint16_t src_port){
	for(int i=0; i<TCP_CONNECTION_LIMIT; i++){
		int ip_check = memcmp(tcp_connection_table_[i].host_ip_addr, ip_addr, 4); 
		int port = (src_port == tcp_connection_table_[i].port) ? 0 : 1; 
		if(!ip_check && !port){
			printf("Found existing connection (ID: %d)\n", i);
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

uint16_t calculate_tcp_checksum(struct ip_header *packet, uint8_t *tcp_segment, uint16_t tcp_len, int verify){

	struct tcp_pseudo_header pseudo;

	memcpy(pseudo.src_addr, packet->src_addr, 4);
	memcpy(pseudo.dest_addr, packet->dest_addr, 4);
	pseudo.zero = 0;
	pseudo.protocol = 6;  // TCP
	pseudo.tcp_len = htons(tcp_len);

	//save original checksum if 

	//uint16_t orig_checksum = *((uint16_t *)(tcp_segment + 16));

	/*
	   if (!verify) {

	   orig_checksum = *((uint16_t*)(tcp_segment + 16));
	// Set checksum field to zero for calculation
	 *((uint16_t*)(tcp_segment + 16)) = 0;
	 }*/


	// Create buffer for pseudo-header + TCP segment
	int total_len = sizeof(struct tcp_pseudo_header) + tcp_len;

	//check for padding
	if (total_len % 2 != 0) {
		total_len++;
	}

	uint8_t *buffer = malloc(total_len);
	if (!buffer) {
		perror("malloc failed");
		return 0;
	}
	memset(buffer, 0, total_len);

	// Copy pseudo-header and TCP segment to buffer
	memcpy(buffer, &pseudo, sizeof(struct tcp_pseudo_header));
	memcpy(buffer + sizeof(struct tcp_pseudo_header), tcp_segment, tcp_len);

	// Calculate checksum over the entire buffer
	uint16_t checksum = ip_checksum(buffer, total_len);

	// Restore original checksum if we're calculating a new one

	/*
	   if (!verify) {
	 *((uint16_t*)(tcp_segment + 16)) = orig_checksum;
	 }*/

	// Clean up
	free(buffer);

	// For debugging, print the original and calculated checksums
	//	printf("TCP Checksum: Original=0x%04x, Calculated=0x%04x\n",ntohs(orig_checksum), ntohs(checksum));

	return htons(checksum);
}

uint32_t get_timestamp(uint32_t client_ts){
	return client_ts++;
}


/*
 * run it everytime handle_tcp_payload is called
 * after read input from user
 * */
void open_connections(struct tcp_connection *tcp_connection_table_){
	int n_connections = 0;

	for(int i=0; i<TCP_CONNECTION_LIMIT; i++){
		if(tcp_connection_table_[i].connection_status){
			n_connections++;
		}
	}

	/*check for open connections*/
	if(n_connections){
		printf("Current open connections: %d\n", n_connections);
		printf("Select and Enter Connection ID to begin conversation\n");
		for(int i=0; i<TCP_CONNECTION_LIMIT; i++){
			if(tcp_connection_table_[i].connection_status){
				printf("%s PORT: %u ID: %d\n", tcp_connection_table_[i].ip_str, tcp_connection_table_[i].port, i);
			}
		}
		return;
	}
	printf("No Connections Open\n");
}

