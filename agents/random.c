#include "game.h"
#include "agents.h"

struct game_state *game_state;

int rand_int(int max) {
	int r, d = RAND_MAX / max;
	max *= d;
	do { r = rand(); } while (r >= max);
	return r / d;
}

struct game_state *play_single_game(void) {
	game_state = headless_init_game();
	while (!game_state->game_over) {
		if (game_state->stats->dropChar != DROP_BLANK) {
			game_state = headless_drop_tile(rand_int(6));
		} else {
			char random_letter = 65 + rand_int(26);
			if (game_state->stats->replace_status == SELECT) {
				int random_tile_ID;
				do {
					random_tile_ID = rand_int(BOARD_WIDTH * BOARD_HEIGHT);
				} while (game_state->board[random_tile_ID] == BOARD_BLANK);
				game_state = headless_replace_tile(
						random_tile_ID, random_letter);
			}
			else if (game_state->stats->dropChar == DROP_BLANK) {
				game_state = headless_assign_blank(random_letter);
			}
		}
		if (game_state == NULL) {
			fprintf(stderr, "most recent move failed");
			break;
		}
	}
	return game_state;
}
