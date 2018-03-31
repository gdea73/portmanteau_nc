#ifndef LETTER_H
#define LETTER_H
#include <stdlib.h>
#include <time.h>

#define CHAR_TABLE_SIZE 110

extern int quantities[];
extern int pointValues[];
extern char charTable[];

void initCharTable(void);

int pointValue(char c);

char saneRandChar(void);

char getNextDropChar(int n_moves);

#endif
