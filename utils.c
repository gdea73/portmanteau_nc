#include "portmanteau.h"

int rand_int(int max) {
	int r, d = RAND_MAX / (max + 1);
	do { r = rand() / d; } while (r > max);
	return r;
}
