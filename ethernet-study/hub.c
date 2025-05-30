
#include <stdio.h>
#include <stdint.h>
#include <sys/random.h>
#include <stdlib.h>
#include "hub_vs_switch.h"
#include <errno.h>
#include <math.h>


int main(int argc, char* argv[]){

	if(argc != 3){
		fprintf(stderr, "invalid number of arguments\n");
		return 1;
	}
	char *endptr, *endptr1; 
	errno = 0;

	long n = strtol(argv[1], &endptr, 10);
	long max_t = strtol(argv[2], &endptr1, 10);

	if(errno != 0 || *endptr != '\0' || n <=0){
		fprintf(stderr, "invalid number\n");
		return 1;
	}
	errno =0;

	if(errno != 0 || *endptr1 != '\0' || max_t <=0){
		fprintf(stderr, "invalid number\n");
		return 1;
	}

	float result  = hub_simulation(n, max_t);
	printf("%f%%\n", result);

	return 0;
}


float hub_simulation(uint32_t n, uint32_t max_t_slot){
	uint32_t t = 0;
	int successful_transmission = 0;
	int attempted_transmission = 0;
	int failed_transmission = 0;
	if(n == 1){
		return 100;
	}

	struct device_config *devices = (struct device_config*) malloc(n*sizeof(struct device_config));

	if (devices == NULL){
		perror("table failed to malloc");
		return 0;
	}

	//assign devices port
	for(int i=0; i<n; i++){
		devices[i].src_port = i;
	}

	while(t < max_t_slot){
		//choose transmitting devices
		for(int i=0; i<n; i++){
			devices[i].transmitting = rand_generator(0, 1);
		}

		//check for collision
		uint32_t *devices_w_collision = malloc(n * sizeof(uint32_t)); // Array of devices that collide
		if (devices_w_collision == NULL) {
			perror("Error: Memory allocation failed");
			free(devices);
			return 0;
		}
		uint32_t number_col_devices = 0; //number of devices that collided
		int col_detection = col_det(devices, devices_w_collision, t, n, &number_col_devices);
		for(int i=0; i<n; i++){
			if(devices[i].transmitting){
				attempted_transmission++;
				if(col_detection){
					//check if device collided
					if(device_col(devices_w_collision, number_col_devices, i)){
						failed_transmission++;
					}

				} else {
					successful_transmission++;
				}
			}
		}

		t++;
	}

	free(devices);

	if(attempted_transmission == 0){
		return 0;
	}

	float r = 100.0f/(float)attempted_transmission;
	return r * (float)successful_transmission;

}

uint32_t rand_generator(uint32_t min, uint32_t max){
	uint32_t buf;	

	if(getrandom(&buf, sizeof(buf), 0) == -1){
		perror("getrandom");
		return 0;
	}
	//scale generated number given range
	return  min + (buf % (max - min + 1));
}

//collision detection
uint32_t col_det(struct device_config *devices_, uint32_t *devices_w_collision_, uint32_t t_, uint32_t n_, uint32_t *number_collision_){
	uint32_t transmitting = 0;
	uint32_t k = 0;
	uint32_t res = 0;

	//check for transmitting devices
	for(int i=0; i<n_; i++){
		if(devices_[i].transmitting){
			transmitting++;
			//transmitting_devices[transmitting++] = i;
		}
	} 

	if (transmitting >= 2){
		// loop through transmitting devices
		for(int i=0; i<n_; i++){
			for(int j=0; j<n_; j++){
				if(i==j)continue;
				if(devices_[i].transmitting && devices_[j].transmitting){
					devices_w_collision_[k++] = i;
					(*number_collision_)++;
					res = 1;

				}
			}
		}
	}
	return res;
}

//check if device collided
uint32_t device_col(uint32_t *devices_w_collision_, uint32_t number_collision_, uint32_t device_id){
	for(int i=0; i< number_collision_; i++){
		if(devices_w_collision_[i] == device_id){
			return 1;
		}
	}
	return 0;
}

