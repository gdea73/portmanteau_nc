#ifndef LETTER_H
#define LETTER_H
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define CHAR_TABLE_SIZE 100
#define DROP_BLANK '*'

extern const int quantities[];
extern const int pointValues[];
extern char charTable[];

void initCharTable(void);

int pointValue(char c);

char getNextDropChar(int n_moves);

#endif
