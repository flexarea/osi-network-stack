#include <stdio.h>
#include <stdint.h>
#include <sys/random.h>
#include <stdlib.h>
#include "binary_exp.h"
#include <errno.h>
#include <math.h>


int main(int argc, char *argv[]){
	if(argc != 2){
		fprintf(stderr, "invalid number of arguments\n");
	}
	char *endptr; 
	errno = 0;

	long n = strtol(argv[1], &endptr, 10);

	if(errno != 0 || *endptr != '\0' || n<=0){
		fprintf(stderr, "invalid number\n");
		return 1;
	}

	uint32_t result  = simulation(n);
	if (result  == 0){
		perror("an error occured");
		return 1;
	}

	printf("%u\n", result);

	return 0;
}

uint32_t simulation(uint32_t n){
	uint32_t t = 0;
	//uint32_t curr_max = 1;
	uint32_t number_completed = 0;

	struct device_config *devices = (struct device_config*) malloc(n*sizeof(struct device_config));
	if (devices == NULL){
		perror("table failed to malloc");
		return 0;
	}

	//populate array with slot 0 attempt
	for(int i=0; i<n; i++){
		devices[i].max_range = 1;
		devices[i].next_attempt = 0;
		devices[i].completed = 0;
		devices[i].curr_max = 1;
	}

	//while loop somewhere here while(number_completed < n){
	while(number_completed < n){
		/*
		if(t >= 10){
			return t;
		}
		*/

		uint32_t devices_w_collision[n]; //array of devices that collide (always >= n)
		uint32_t number_col_devices = 0; //number of devices that collided


		int col_detection = col_det(devices, devices_w_collision, t, n, &number_col_devices); //check for collision & record collided devices

		//printf("Collided Devices: %u\n", number_col_devices);
		for(int i=0; i<n; i++){
			if(devices[i].completed) continue;

			//check for collison
			if(devices[i].next_attempt == t){
				if(col_detection){

					//check if device collided
					//if there is collision then the non-collided device doesn't send at this time slot

					if(device_col(devices_w_collision, number_col_devices, i)){
						uint32_t rand_n = rand_generator(0, devices[i].max_range);
						devices[i].next_attempt = (t + rand_n) + 1; //record next attempt 
						//printf("next_attempt updated for device %u\n", i);
						devices[i].curr_max++;
						devices[i].max_range = (1 << devices[i].curr_max) - 1; //update range
					}

				}else{

					//No collision. can send frame in this time slot
					devices[i].completed = 1;	
					number_completed++;
				}
			}
			//printf("Device %u: new max_range=%u, next_attempt=%u\n", i, devices[i].max_range, devices[i].next_attempt);
		}

		//printf("Elapsed Time: %u, Completed: %u/%u\n", t, number_completed, n);
		//printf("-----------------------------------------\n");
		t++; //move to next timeslot
	}

	free(devices);

	return t;
	//ends here
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
	//check for transmitting devices
	for(int i=0; i<n_; i++){
		if(devices_[i].next_attempt == t_ && !devices_[i].completed){
			transmitting++;
		}

	} 

	if (transmitting >= 2){
		for(int i=0; i<n_; i++){
			if(devices_[i].next_attempt == t_ && !devices_[i].completed){
				devices_w_collision_[k] = i;
				k++;
				(*number_collision_)++;
			}

		}
		return 1;
	}

	return 0;
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
