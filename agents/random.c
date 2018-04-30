#include "agents.h"

struct game *random_play_single_game(void) {
	initCharTable();
	struct game *g = init_game(1);
	set_game(g);
	while (!g->game_over) {
		if (g->replace_status == NORMAL
		    && g->drop_letter != DROP_BLANK) {
			// normal operation: drop tile into a random column
			headless_drop_tile(rand_int(6));
		} else {
			// either a blank or a replacement: come up with a random letter
			char random_letter = 65 + rand_int(25);
			if (g->replace_status == SELECT) {
				// replace a random tile with the random letter assigned above
				int rand_tile_ID;
				do {
					rand_tile_ID = rand_int(BOARD_WIDTH * BOARD_HEIGHT);
				} while (
					g->board[rand_tile_ID / 7][rand_tile_ID % 7] == BOARD_BLANK
				);
				headless_replace_tile(
					rand_tile_ID, random_letter
				);
			} else if (g->drop_letter == DROP_BLANK) {
				// assign the random letter assigned above to the blank
				headless_assign_blank(random_letter);
			}
		}
		if (g == NULL) {
			fprintf(stderr, "most recent move failed");
			break;
		}
	}
	return g;
}
