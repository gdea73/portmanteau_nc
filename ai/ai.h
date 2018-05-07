#ifndef AI_H
#define AI_H

#define STRATEGY_NAME_MAX_LENGTH 80

// constants regarding heuristic function h2() (and h3())
#define COL_SCORE_LO -100
#define COL_SCORE_HI 80
#define WORD_SCORE_HI 1000
// the higher this ratio, the greater the AI values building
// high-scoring words relative to keeping the board clear
#define COL_SCORE_SCALE COL_SCORE_HI / (float) WORD_SCORE_HI
#define COL_SCORE_SHORT 70

#include "game.h"
#include "utils.h"

struct dictionary *mdict;
int verbosity_level, n_games;

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

void print_board(char board[7][7]);

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
// greedy strategies -- heuristic 3 (h1 with fallback to h2)
int get_greedy_normal_move_h3(struct game *game);
struct blank_move get_greedy_blank_move_h3(struct game *game);
struct replace_move get_greedy_replace_move_h3(struct game *game);

int get_column_normal_move(struct game *game);

int get_sequential_normal_move(struct game *game);

void play_AI_game(struct agent agent, struct game *game);

int main(int argc, char **argv);

#endif
