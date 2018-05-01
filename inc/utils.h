#ifndef UTILS_H
#define UTILS_H

#define ARRAY_SIZE(A) sizeof(A) / sizeof(A[0])

int rand_int(int max);

void obfuscate_tile_set(char *tile_set, int n_moves);

#endif
