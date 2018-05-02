#include "ai.h"
#include "game.h"

int get_random_normal_move(struct game *game) {
	// degenerate case: ignores game state entirely and picks a random column
	return rand_int(7);
}

struct blank_move get_random_blank_move(struct game *game) {
	struct blank_move move;
	move.letter = 65 + rand_int(25);
	move.drop_col = rand_int(7);
	return move;
}

struct replace_move get_random_replace_move(struct game *game) {
	struct replace_move move;
	move.letter = 65 + rand_int(25);
	do {
		move.tile_ID = rand_int(7 * 7);
	} while (game->board[move.tile_ID / 7][move.tile_ID % 7] == BOARD_BLANK);
	return move;
}

