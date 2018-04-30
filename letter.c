#include "letter.h"

int charTablePopulated = 0;
// initialized once, to keep track of letter distribution
char charTable[CHAR_TABLE_SIZE];

const int pointValues[] = {
 // A, B, C, D, E, F, G, H, I, J, K, L, M,
	1, 3, 3, 2, 1, 4, 2, 4, 1, 8, 5, 1, 3,
 // N, O, P, Q , R, S, T, U, V, W, X, Y, Z
 	1, 1, 3, 10, 1, 1, 1, 1, 4, 4, 8, 4, 10
};

const int quantities[] = {
 // A, B, C, D, E , F, G, H, I, J, K, L, M,
	9, 2, 2, 4, 12, 2, 3, 2, 8, 1, 1, 4, 2,
 // N, O, P, Q, R, S, T, U, V, W, X, Y, Z
 	6, 8, 2, 1, 6, 4, 6, 4, 2, 2, 1, 2, 1
 // leaves room for 3 blanks (of 100)
};

int pointValue(char c) {
	if (c < 65 || c > 90) {
		// this includes the 'blank' (asterisk)
		return 0;
	}
	return pointValues[c - 65];
}

void initCharTable(void) {
	int i = 0, j = 0, k = 0;
	while (i < 26) {
		while (k < quantities[i]) {
			charTable[j] = i + 65;
			k++; j++;
		}
		i++; k = 0;
	}
	while (j < CHAR_TABLE_SIZE) {
		charTable[j] = DROP_BLANK;
		j++;
	}
	charTablePopulated = 1;
}

void generate_tile_set(char *tile_set) {
	// generate a new tile set
	int i, j;
	char temp;
	for (i = 0; i < CHAR_TABLE_SIZE; i++) {
		tile_set[i] = charTable[i];
	}
	for (i = CHAR_TABLE_SIZE - 1; i >= 0; i--) {
		// randomly swap characters to shuffle the drop tile set
		j = (int) floor(rand() / (RAND_MAX / (i + 1)));
		temp = tile_set[i];
		tile_set[i] = tile_set[j];
		tile_set[j] = temp;
	}
}

char get_next_drop_letter(struct game *game) {
	if (!charTablePopulated) {
		initCharTable();
	}
	if (game->n_moves % CHAR_TABLE_SIZE == 0) {
		generate_tile_set(game->tile_set);
	}
	return game->tile_set[game->n_moves % CHAR_TABLE_SIZE];
}
