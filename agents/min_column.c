#include "agents.h"

struct game *min_column_play_single_game(void) {
	initCharTable();
	struct game *g = init_game(1);
	set_game(g);
	// initialize column heights to 0
	while (!g->game_over) {
		if (g->replace_status == NORMAL
		    && g->drop_letter != DROP_BLANK) {
			// normal operation: determine how many letters are in each column
			// indices 8-14 are used to indicate shortest columns
			int column_height[14] = {0};
			for (int i = 0; i < BOARD_WIDTH; i++) {
				for (int j = 0; j < BOARD_HEIGHT; j++) {
					if (g->board[i][j] != BOARD_BLANK) {
						column_height[i]+=1;
					}
				}
			}
			// identify columns with fewest letters
			int shortest = 7;
			for (int i = 0; i < 7; i++) {
				if (column_height[i] <= shortest) {
					shortest = column_height[i];
				}
			}
			// indicate the columns
			for (int i = 0; i < 7; i++) {
				if (column_height[i] == shortest) {
					column_height[i+7] = 1;
				}
			}
			// pick randomly from columns containing fewest letters
			int column_choice = rand_int(6);
			int column_check = 0;
			while (column_check == 0) {
				if (column_height[column_choice+7] == 1) {
					column_check = 1;
				}
				column_choice = rand_int(6);
			}
			headless_drop_tile(column_choice);
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
