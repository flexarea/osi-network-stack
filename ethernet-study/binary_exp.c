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

	ssize_t result  = simulation(n);
	if (result  == 0){
		perror("an error occured");
		return 1;
	}

	printf("%zd\n", result);

	return 0;
}

ssize_t simulation(ssize_t n){
	ssize_t t = 0;
	ssize_t curr_max = 1;
	ssize_t number_completed = 0;
	
	struct device_config *devices = (struct device_config*) malloc(n*sizeof(struct device_config));
	if (devices == NULL){
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


		int col_detection = col_det(devices, devices_w_collision, t, n, &number_col_devices); //check for collision & record collided devices

		for(ssize_t i=0; i<n; i++){
			if(devices[i].completed) continue;

			//check for collison
			if(col_detection){

				//check if device collided
				//if there is collision then the non-collided device doesn't send at this time slot

				if(device_col(devices_w_collision, number_col_devices, i)){
					ssize_t rand_n = rand_generator(0, devices[i].max_range);
					devices[i].next_attempt = t + rand_n + 1; //record next attempt 
					devices[i].max_range = pow(2, curr_max) - 1;
					curr_max++;
				}

			}else{

				//No collison. can send frame in this time slot
				if(devices[i].next_attempt == t){
					devices[i].completed = 1;	
					number_completed++;
				}
			}
		}
		t++; //move to next timeslot
	}

	free(devices);

	return t;
	//ends here
}
ssize_t rand_generator(ssize_t min, ssize_t max){
	 ssize_t buf;	

	if(getrandom(&buf, sizeof(buf), 0) == -1){
		perror("getrandom");
			return 0;
	}

	//scale generated number given range
	return  min + (buf % (max - min + 1));
}

//collision detection
ssize_t col_det(struct device_config *devices_, ssize_t *devices_w_collision_, ssize_t t_, ssize_t n_, ssize_t *number_collision_){
	ssize_t k=0;
	ssize_t is_collision = 0;
	for(ssize_t i=0; i<n_; i++){
			if(devices_[i].completed) continue;
		for(ssize_t j=i+1; j<n_; j++){
			if(devices_[i].completed) continue;

			//if device has same slot with 1 other device then append it to array of collided device and move to next
			if(devices_[i].next_attempt == devices_[j].next_attempt && devices_[i].next_attempt == t_){
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
ssize_t device_col(ssize_t *devices_w_collision_, ssize_t number_collision_, ssize_t device_id){
	for(ssize_t i=0; i< number_collision_; i++){
		if(devices_w_collision_[i] == device_id){
			return 1;
		}
	}
	return 0;
}
