#include "ai.h"
#include "game.h"

int column = -1;
int get_sequential_normal_move(struct game *game) {
	column+=1;
	return column%7;
}
