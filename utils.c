#include "portmanteau.h"
#include "letter.h"

int rand_int(int max) {
	int r, d = RAND_MAX / (max + 1);
	do { r = rand() / d; } while (r > max);
	return r;
}

void obfuscate_tile_set(char *tile_set, int n_moves) {
	int index = n_moves % CHAR_TABLE_SIZE + 1, i, j;
	char temp;
	// the agent knows the tile set up to this point,
	// but it is illegal for it to know the future drop letters
	for (i = CHAR_TABLE_SIZE - 1; i >= index; i--) {
		j = (int) floor(rand_int(i - index) + index);
		temp = tile_set[i];
		tile_set[i] = tile_set[j];
		tile_set[j] = temp;
	}
}
