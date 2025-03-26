#include <stdio.h>
#include <stdint.h>
#include <sys/random.h>
#include <stdlib.h>
#include "binary_exp.h"

int main(int argc, char *argv[]){
	return 0;
}

ssize_t simulation(ssize_t n){
	ssize_t t = 0;
	ssize_t curr_min = 0;
	ssize_t curr_max = 1;
	unsigned int gen_n;  //generated number	
	ssize_t completed[n];
	ssize_t cmp_idx = 0;
	ssize_t capacity = 1;


	ssize_t** table = (ssize_t**) malloc(n*sizeof(ssize_t));
	if (table == NULL){
		perror("table failed to malloc");
		return 0;
	}
	//allocate memory for each rows
	for(int i=0; i<n; i++){
		table[i] = (ssize_t*) malloc(capacity*sizeof(ssize_t));
		if (table[i] == NULL){
			perror("table failed to malloc");
			return 0;
		}
	}

	//populate array with slot 0 attempt
	for(int i=0; i<n; i++){
		table[i][t] = 0;
	}

	//while loop somewhere here
	while(1){
		if(cmp_idx == (n-1)){
			break;
		}
		ssize_t col_dev[n]; //array of devices that collide (always >= n)
						   //detect collision
		ssize_t cdev_len = 0; //number of devices that collided
		ssize_t pt = ((t-1) < 0) ? 0 : t-1; //previous tslot
		int col_det = (table, col_dev, pt, n, &cdev_len); //check for collision & record collided devices
		for(int i=0; i<n; i++){
			if(col_det){
				//check if device collided
				if(in_col_dev(col_dev, cdev_len, i) == 1){  //if there is collision then the non-collided device doesn't send at this time slot
					ssize_t rand_n = rand_generator(curr_min, curr_max);
					ssize_t next_attempt = t + rand_n + 1;
					table[i][t] = next_attempt;

				}else{
					table[i][t] = table[i][pt];
				}


			}else{
				//can send frame in this time slot
				if(table[i][pt] == t){
					completed[++cmp_idx] = i;	
				}else{
					//wait for next attempt
					table[i][t] = table[i][pt];
				}
			}

			//increase buffer size
			if(t >= capacity){
				capacity *= 2;
				ssize_t *ptr = (ssize_t*) realloc(capacity * sizeof(ssize_t))
				if(ptr == NULL){
					perror("realloc faild");
					for(int i=0; i<n; i++){
						free(table[i]);
					}
					free(table);
					return 0;
				}

				table[i] = ptr;
			}
		}
		t++; //move to next timeslot
	}

	for(int i=0; i<n; i++){
		free(table[i]);
	}
	free(table);
	return t;
	//ends here
}
ssize_t rand_generator(ssize_t min, ssize_t max){
	unsigned int buf;	

	if(getrandom(buf, sizeof(buf), 0) == -1){
		perror("getrandom")
			return 0;
	}

	//scale generated number given range
	return  min + ((ssize_t)buf % (max - min + 1));
}

//collision detection
int col_det(ssize_t *table_, ssize_t *col_dev_, ssize_t t_, ssize_t n_, ssize_t *cdev_len_){
	ssize_t k=0;
	int is_collision = 0;
	for(ssize_t i=0; i<n_, i++){
		for(ssize_t j=0; j<n; j++){
			if(j == i){
				continue;
			}
			//if device has same slot with 1 other device then append it to array of collided device and move to next
			if(table_[i][t_] == table[j][t_] && table[i][t_] == t_+1){
				is_collision = 1;
				col_dev[k] = i;
				k++;
				*cdev_len_++;
				break;
			}
		}
	} 
	return is_collision;
}
//check if device collided
int in_col_dev(ssize_t *col_dev_, ssize_t *cdev_len_, ssize_t dev_idx){
	for(int i=0; i<cdev_len; i++){
		if(col_dev[i] == dev_idx){
			return 1;
		}
	}
	return 0;
}
