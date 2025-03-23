#include <stdio.h>
#include <stdint.h>
#include <sys/random.h>

int main(int argc, char *argv[]){
	return 0;
}

ssize_t simulation(ssize_t n){
	ssize_t t = 0;
	ssize_t curr_min = 0;
	ssize_t curr_max = 1;
	unsigned int gen_n;  //generated number	
	ssize_t table[n][];
	ssize_t col_dev[];

	//populate array with slot 0 attempt
	for(int i=0; i<n; i++){
		table[i][t] = 0;
	}

	//while loop somewhere here
	while(1){
		//detect collision
		ssize_t cdev_len = 0; //number of devices that collided
		if(col_det(table, col_dev, t, n) == 1) {
			//generate random number for reach device
			for(int i=0; i<n; i++){
				if(in_col_dev(col_dev, cdev_len, i) == 1){
					ssize_t rand_n = rand_generator(curr_min, curr_max);
					ssize_t next_attempt = t + rand_n + 1;
					table[i][t] = next_attempt;
				}else{
					table[i][t] = table[i][t-1];
				}
			}

			
		}

		

		//the end
		t++; //move to next timeslot
	}
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
			//if device has same slot with 1 other device then append it to list of collided device and move to next
			if(table_[i][t_] == table[j][t_]){
				is_collision = 1;
				col_dev[k] = i;
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
}

