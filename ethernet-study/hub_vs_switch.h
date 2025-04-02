
#ifndef HUB_VS_SWITCH_H
#define HUB_VS_SWITCH_H

#include <sys/types.h>
#include <stdint.h>

typedef struct device_config{
	uint32_t src_port;	
	uint32_t dst_port;
	uint32_t transmitting;
}device_config;

uint32_t rand_generator(uint32_t min, uint32_t max);
uint32_t col_det(struct device_config *devices, uint32_t *devices_w_collision, uint32_t t_, uint32_t n_, uint32_t *number_collision_);
uint32_t device_col(uint32_t *devices_w_collision, uint32_t number_collision_, uint32_t device_id);
float switch_simulation(uint32_t n, uint32_t max_t_slot);
float hub_simulation(uint32_t n, uint32_t max_t_slot);

#endif
