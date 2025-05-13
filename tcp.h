#ifnef TCP_H
#define TCP_H

#include <sys/types.h>
#include <stdint.h>
#include <arpa/inet.h>

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

typedef struct connections{
	uint8_t connection_id;
	uint8_t external_interface_ip[4];
	uint8_t connection_status; //active, 1 (waiting for syn-ack), 2 (waiting for ack)
}

#endif
