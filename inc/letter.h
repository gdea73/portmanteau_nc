#ifndef LETTER_H
#define LETTER_H
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "game_struct.h"

#define CHAR_TABLE_SIZE 100
#define DROP_BLANK '*'

extern const int quantities[];
extern const int pointValues[];
extern char charTable[];

void initCharTable(void);

int pointValue(char c);

void generate_tile_set(char *tile_set);

char get_next_drop_letter(struct game *game);

#endif
