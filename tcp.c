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


void handle_tcp(ssize_t len, struct frame_fields *frame_f, uint8_t *or_frame, struct ip_header *packet, struct packet_info *packet_inf, struct icmp *curr_icmp, struct interface *interface_list_, int transmitter_id){
	//checksum?
	int connection = 0;
	uint8_t tcp_ip_addr[] = TCP_IP; //interface server
	struct tcp *tcp_header = (struct tcp *) or_frame + 20; //extract tcp payload

	uint8_t control_bits = ntohs(tcp_header->flag) & 0x3F;  //bitmask to extract flags only
	uint8_t data_offset = (ntohs(tcp_header->flag) >> 12) & 0xF; //extract data offset
	uint8_t tcp_payload_len = data_offset*4;
	uint8_t data_octets = 0;

	if((tcp_payload_len+14+20) == len){
		data_octets = 1;
	}else{
		data_octets = len - (tcp_payload_len+14+20)
	}

	if(memcmp(tcp_ip_addr, packet->dest_addr, 4) != 0){
		printf("Dropping packet: unknown TCP IP address\n");
		return;
	}

	//verify if port match
	if(ntohs(tcp_header->dest_port) != HOST_TCP_PORT){
		printf("Dropping packet: unknown port\n");
		return;
	}
	connection_status = tcp_reply(tcp_header, control_bits, data_octets, data_offset);
	
	while()


}
//how do we know we are receiving the first syn (ack == 0?)

//create sequence number, modify ack number, send tcp packet
int tcp_reply(struct tcp *tcp_header_, uint8_t control_bits, int8_t data_octets, int8_t data_offset_){
	uint8_t forward_flag = 0;
	//check  flag
	switch(control_bits){
	//syn(2), ack(16), fin(1), syn/ack(18), ack_fin(10), psh(3)
		case 2: 
			/*handle syn*/
			tcp_header_->ack_number = (seq_number+data_octets); //create ack
			//generate seq number
			tcp_header_->seq_number = tcp_seq_generator(tcp_header_->seq_number);
			//set flag to syn/ack
			tcp_header_->flag = htons((data_offset_ << 12) | 0x12);
			//update port
			tcp_header_->dest_port = tcp_header_->src_port;
			tcp_header_->src_port = htons(HOST_TCP_PORT);
		case 16:
			//handle ack
			break;
		case 18:
			//handle syn+ack
			break;
		case 10:
			//handle fin+ack
			break;
		case 1:
			//handle fin
			break;
		default:
			printf("unsupported flag");
			return
	}

}

//calculate data length

//handle action connection
void handle_active_connection(){
}

//check connection table 
int check_connection_status(){
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
