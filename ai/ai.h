#ifndef AI_H
#define AI_H

#define STRATEGY_NAME_MAX_LENGTH 80

#include "game.h"
#include "utils.h"

struct dictionary *mdict;
int verbosity_level;

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

// random strategies
int get_random_normal_move(struct game *game);
struct blank_move get_random_blank_move(struct game *game);
struct replace_move get_random_replace_move(struct game *game);
// greedy strategies -- heuristic 1 (score)
int get_greedy_normal_move_h1(struct game *game);
struct blank_move get_greedy_blank_move_h1(struct game *game);
struct replace_move get_greedy_replace_move_h1(struct game *game);
// greedy strategies -- heuristic 2 ("column scores")
int get_greedy_normal_move_h2(struct game *game);
struct blank_move get_greedy_blank_move_h2(struct game *game);
struct replace_move get_greedy_replace_move_h2(struct game *game);

int get_column_normal_move(struct game *game);

int get_sequential_normal_move(struct game *game);

void play_AI_game(struct agent agent, struct game *game);

int main(int argc, char **argv);

#endif
