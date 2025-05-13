#ifndef TCP_H
#define TCP_H

#include <sys/types.h>
#include <stdint.h>
#include <arpa/inet.h>

//stack.h and ip.h struct declarations

struct ip_header;
struct packet_info;

#define TCP_NONE -1 // No transmission
#define TCP_REG 0  // Regular (single) transmission
#define TCP_FIN_ACK 1 // Double transmission

typedef struct tcp{
	uint16_t src_port;
	uint16_t dest_port;
	uint32_t seq_number;
	uint32_t ack_number;
	uint16_t flag; //000000
	uint16_t window;
	uint16_t checksum;
	uint16_t urgent_pointer;
}tcp;


typedef struct tcp_connection{
	int extreme; //1(connecting) 0(listening)
	int connection_id;
	uint8_t host_ip_addr[4];
	char ip_str[INET_ADDRSTRLEN];
	uint32_t next_seq;
	uint32_t curr_seq;
	int connection_status; //active(1), SYN/RCVD(2) not_connected(0) FIN/ACK(3)
}tcp_connection;


int handle_tcp(ssize_t len,  uint8_t *or_frame, struct ip_header *packet, struct packet_info *packet_inf, struct tcp_connection *tcp_connection_table_, int flag);

int handle_tcp_payload(uint8_t *or_frame, struct tcp *tcp_header_, struct packet_info *packet_inf, uint8_t control_bits, int8_t data_octets, int8_t data_offset_, struct tcp_connection *tcp_connection_table_, struct ip_header *packet, int connection_id, int flag, uint8_t tcp_header_len);

int connection_lookup(struct tcp_connection *tcp_connection_table_, uint8_t *ip_addr);
int is_space(struct tcp_connection *tcp_connection_table_);

uint32_t tcp_seq_generator(uint32_t prev);

uint16_t calculate_tcp_checksum(struct ip_header *packet, uint8_t *tcp_header, uint16_t tcp_payload_len);
#endif
