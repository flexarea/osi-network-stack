#include <sys/types.h>

char *binary_to_hex(void *data, ssize_t  n);
void *hex_to_binary(char *hex, ssize_t *bin_bytes);
int get_size(char *hex);
char upper_case(char c);
int non_hex(char x);
