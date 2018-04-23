#include "agents.h"

struct game_state *sequential_play_single_game(void) {
	initCharTable();
	struct game_state *current_game = malloc(sizeof(struct game_state));
	current_game = headless_init_game();
	int col = 0;
	while (!current_game->game_over) {
		if (current_game->stats->replace_status == NORMAL
		    && current_game->stats->dropChar != DROP_BLANK) {
			// normal operation: drop tile into a random column
			current_game = headless_drop_tile(col++);
		} else {
			// either a blank or a replacement: come up with a random letter
			char random_letter = 65 + rand_int(25);
			if (current_game->stats->replace_status == SELECT) {
				// replace a random tile with the random letter assigned above
				int random_tile_ID;
				do {
					random_tile_ID = rand_int(BOARD_WIDTH * BOARD_HEIGHT);
				} while (current_game->board[random_tile_ID] == BOARD_BLANK);
				current_game = headless_replace_tile(
					random_tile_ID, random_letter
				);
			} else if (current_game->stats->dropChar == DROP_BLANK) {
				// assign the random letter assigned above to the blank
				current_game = headless_assign_blank(random_letter);
			}
		}
		if (current_game == NULL) {
			fprintf(stderr, "most recent move failed");
			break;
		}
		col %= BOARD_WIDTH;
	}
	return current_game;
}
