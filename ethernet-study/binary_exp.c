#include <stdio.h>
#include <stdint.h>
#include <sys/random.h>
#include <stdlib.h>
#include "binary_exp.h"


int main(int argc, char *argv[]){
	ssize_t start_simulation = simulation((ssize_t)argv[1]);
	if (start_simulation == 0){
		perror("an error occured");
	}

	return (int)start_simulation;
}

ssize_t simulation(ssize_t n){
	ssize_t t = 0;
	ssize_t curr_max = 1;
	ssize_t number_completed = 0;
	
	//populate max range for each devices
	for(ssize_t i=0; i<n; i++){
		backoff_range_arr[i] = curr_max;
	}

	struct device_config *devices = (struct device_config*) malloc(n*sizeof(struct device_config));
	if (table == NULL){
		perror("table failed to malloc");
		return 0;
	}

	//populate array with slot 0 attempt
	for(ssize_t i=0; i<n; i++){
		devices[i].max_range = 1;
		devices[i].next_attempt = 0;
		devices[i].completed = 0;
	}

	//while loop somewhere here
	while(1){
		if(number_completed == n){
			break;
		}
		ssize_t devices_w_collision[n]; //array of devices that collide (always >= n)
		ssize_t number_col_devices = 0; //number of devices that collided

		/*
		 * TODO: remove if unnecessary
		ssize_t pt = ((t-1) < 0) ? 0 : t-1; //previous tslot
		*/

		int col_detection = col_det(table, devices_w_collision, t, n, &number_collided_devices); //check for collision & record collided devices

		for(ssize_t i=0; i<n; i++){
			//check for collison
			if(col_detection){

				//check if device collided
				//if there is collision then the non-collided device doesn't send at this time slot

				if(device_col(col_dev, number_col_devices, i)){
					ssize_t rand_n = rand_generator(0, table[i].max_range);
					table[i].next_attempt = t + rand_n + 1; //record next attempt 
					table[i].max_range = (2**curr_max) - 1;
				}

			}else{

				//No collison. can send frame in this time slot
				if(table[i].next_attempt == t && !table[i].completed){
					table[i].completed = 1;	
					number_completed++;
				}
			}
		}
		t++; //move to next timeslot
	}

	for(ssize_t i=0; i<n; i++){
		free(table[i]);
	}

	return t;
	//ends here
}
ssize_t rand_generator(ssize_t min, ssize_t max){
	 ssize_t buf;	

	if(getrandom(buf, sizeof(buf), 0) == -1){
		perror("getrandom")
			return 0;
	}

	//scale generated number given range
	return  min + (buf % (max - min + 1));
}

//collision detection
ssize_t col_det(ssize_t *table_, ssize_t *devices_w_collision_, ssize_t t_, ssize_t n_, ssize_t *number_collision_){
	ssize_t k=0;
	ssize_t is_collision = 0;
	for(ssize_t i=0; i<n_, i++){
		for(ssize_t j=0; j<n; j++){
			if(j == i){
				continue;
			}
			//if device has same slot with 1 other device then append it to array of collided device and move to next
			if(table_[i].next_attempt == table[j].next_attempt && table[i].next_attempt == t){
				is_collision = 1;
				devices_w_collision_[k] = i;
				k++;
				*number_collision_++;
				break;
			}
		}
	} 
	return is_collision;
}
//check if device collided
ssize_t device_col(ssize_t *devices_w_collision_, ssize_t *number_collision_, ssize_t device_id){
	for(ssize_t i=0; i<number_collision_; i++){
		if(devices_w_collision[i] == device_id){
			return 1;
		}
	}
	return 0;
}
