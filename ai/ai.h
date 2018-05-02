#ifndef AI_H
#define AI_H

#define STRATEGY_NAME_MAX_LENGTH 80

#include "game.h"
#include "utils.h"

struct blank_move {
	char letter;
	int drop_col;
};

struct replace_move {
	char letter;
	int tile_ID;
};

struct agent {
	int (*get_normal_move)(struct game *game);
	struct blank_move (*get_blank_move)(struct game *game);
	struct replace_move (*get_replace_move)(struct game *game);
	const char *strategy_name;
};

int get_random_normal_move(struct game *game);
struct blank_move get_random_blank_move(struct game *game);
struct replace_move get_random_replace_move(struct game *game);

void play_AI_game(struct agent agent, struct game *game);

int main(int argc, char **argv);

#endif
