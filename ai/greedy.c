#include "ai.h"
#include "game.h"

int get_greedy_normal_move(struct game *g) {
	static struct game sim;
	int best_col, i, best_score = g->score;
	best_col = rand_int(7);
	set_game(&sim);
	for (i = 0; i < 7; i++) {
		if (g->board[i][0] == BOARD_BLANK) {
			memcpy(&sim, g, sizeof(struct game));
			headless_drop_tile(i);
			if (sim.score > best_score) {
				best_col = i;
				best_score = sim.score;
			}
		}
	}
	set_game(g);
	return best_col;
}

struct blank_move get_greedy_blank_move(struct game *g) {
	static struct game sim;
	char c;
	int i, best_score = g->score;
	struct blank_move best_move = {'A', 3}; // default move in case all equal
	set_game(&sim);
	for (i = 0; i < 7; i++) {
		if (g->board[i][0] == BOARD_BLANK) {
			for (c = 'A'; c <= 'Z'; c++) {
				memcpy(&sim, g, sizeof(struct game));
				headless_assign_blank(c);
				headless_drop_tile(i);
				if (sim.score > best_score) {
					best_score = sim.score;
					best_move.letter = c;
					best_move.drop_col = i;
				}
			}
		}
	}
	// done simulating: restore normal game state
	set_game(g);
	return best_move;
}

struct replace_move get_greedy_replace_move(struct game *g) {
	static struct game sim;
	char c;
	int tile_ID, best_score = g->score;
	struct replace_move best_move = { 0 };
	set_game(&sim);
	for (tile_ID = 0; tile_ID < 7 * 7; tile_ID++) {
		if (g->board[tile_ID / 7][tile_ID % 7] != BOARD_BLANK) {
			for (c = 'A'; c <= 'Z'; c++) {
				memcpy(&sim, g, sizeof(struct game));
				headless_replace_tile(tile_ID, c);
				if (sim.score >= best_score) {
					best_move.tile_ID = tile_ID;
					best_move.letter = c;
				}
			}
		}
	}
	// done simulating: restore normal game state
	set_game(g);
	return best_move;
}
