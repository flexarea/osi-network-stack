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
#define TCP_FIN_ACK -3 // Double transmission

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

typedef struct tcp_pseudo_header {
    uint8_t src_addr[4];
    uint8_t dest_addr[4];
    uint8_t zero;
    uint8_t protocol;
    uint16_t tcp_len;
} tcp_pseudo_header;

typedef struct tcp_connection{
	int io; //1(write mode) 0(read mode)
	uint8_t host_ip_addr[4];
	uint8_t host_mac[6];
	char ip_str[INET_ADDRSTRLEN];
	uint16_t port;
	uint32_t next_seq; //next server's ack number
	uint32_t curr_seq; //next server's sequence  number
	int connection_status; //active(1), SYN/RCVD(2) not_connected(0)
	int fin_ack;
}tcp_connection;


/*general tcp functions*/
int handle_tcp(ssize_t len, struct frame_fields *frame_f, uint8_t *or_frame, struct ip_header *packet, struct packet_info *packet_inf, struct tcp_connection *tcp_connection_table_, int flag);

int handle_tcp_payload(uint8_t *or_frame, struct tcp *tcp_header_, struct packet_info *packet_inf, uint8_t control_bits, int8_t data_octets, int8_t data_offset_, struct tcp_connection *tcp_connection_table_, struct ip_header *packet, int connection_id, int flag, uint8_t tcp_header_len, struct frame_fields *frame_f);

int connection_lookup(struct tcp_connection *tcp_connection_table_, uint8_t *ip_addr, uint16_t src_port);
int is_space(struct tcp_connection *tcp_connection_table_);

uint32_t tcp_seq_generator(uint32_t prev);

uint16_t calculate_tcp_checksum(struct ip_header *packet, uint8_t *tcp_header, uint16_t tcp_payload_len, int verify);
void open_connections(struct tcp_connection *tcp_connection_table_);

/*Interaction functions*/
int parse_connection_command(char *input, char *command);
int create_tcp_segment(int data_len, char *buffer, struct tcp_connection *tcp_connection_table_, uint8_t *tcp_mac, uint8_t *tcp_ip, int device_id, uint8_t **out_frame, int *out_frame_size, int fin_flag);
int read_user_input(char *buffer, int buffer_size);
int promise(struct tcp_connection *tcp_connection_table_);
int interaction_handler(int interface_id, struct tcp_connection *tcp_connection_table_, uint8_t *tcp_mac, uint8_t *tcp_ip, struct interface *interface_list_);
int verify_user_id_input(int device_id, struct tcp_connection *tcp_connection_table_);

/*other stuff*/
uint32_t get_timestamp(uint32_t client_ts);
#endif
