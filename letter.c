#include "letter.h"

int charTablePopulated = 0;
char charTable[CHAR_TABLE_SIZE];

int pointValues[] = {
 // A, B, C, D, E, F, G, H, I, J, K, L, M,
	1, 3, 3, 2, 1, 4, 2, 4, 1, 8, 5, 1, 3,
 // N, O, P, Q , R, S, T, U, V, W, X, Y, Z
 	1, 1, 3, 10, 1, 1, 1, 1, 4, 4, 8, 4, 10
};

int quantities[] = {
 // A, B, C, D, E , F, G, H, I, J, K, L, M,
	9, 2, 2, 4, 12, 2, 3, 2, 9, 1, 1, 4, 2,
 // N, O, P, Q, R, S, T, U, V, W, X, Y, Z
 	6, 8, 2, 1, 6, 4, 6, 4, 2, 2, 1, 2, 1
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
		charTable[j] = '*';
		j++;
	}
	charTablePopulated = 1;
}

// TODO: better randomness
char saneRandChar(void) {
	static int initialized = 0;
	if (!initialized) {
		srand((unsigned int) time(NULL));
	}
	if (!charTablePopulated) {
		initCharTable();
	}
	return charTable[rand() % CHAR_TABLE_SIZE];
	// return (rand() % 26) + 65;
}
