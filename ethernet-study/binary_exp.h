#ifndef BINARY_EXP_H
#define BINARY_EXP_H

#include <sys/types.h>
#include <stdint.h>

ssize_t simulation(ssize_t n);
ssize_t rand_generator(ssize_t min, ssize_t max);
int col_det(ssize_t *table_, ssize_t *col_dev_, ssize_t t_, ssize_t n_, ssize_t *cdev_len_);
int in_col_dev(ssize_t *col_dev_, ssize_t *cdev_len_, ssize_t dev_idx);

#endif
