#ifndef BINARY_EXP_H
#define BINARY_EXP_H

#include <sys/types.h>
#include <stdint.h>

typedef struct device_config{
	ssize_t max_range;	
	ssize_t next_attempt;
	ssize_t completed;
}device_config;

ssize_t simulation(ssize_t n);
ssize_t rand_generator(ssize_t min, ssize_t max);
int col_det(ssize_t *table_, ssize_t *devices_w_collision, ssize_t t_, ssize_t n_, ssize_t *number_collision_);
int device_col(ssize_t *devices_w_collision, ssize_t *number_collision_, ssize_t device_id);

#endif
