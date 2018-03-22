#ifndef WORDS_H
#define WORDS_H

#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define WORDSBS 127
#define SCORE_MULTIPLIER 7

struct dictionary {
	char **words;
	int length;
};

struct boardWord {
	int startCol;
	int startRow;
	int endCol;
	int endRow;
};

struct dictionary *loadDict(char *filename);

void freeDict(struct dictionary *dict);

uint8_t isValidWord(struct dictionary *dict, char *sz);

uint8_t binSearch(char **words, char *query, size_t startIdx, size_t endIdx);

int wordScore(char *word);

char *reverse(char *s);

#endif
