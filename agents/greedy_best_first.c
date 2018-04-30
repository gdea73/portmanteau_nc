#include "agents.h"

int get_greedy_best_drop_col(struct game *g) {
	static struct game sim;
	int best_col, best_score = 0, i;
	set_game(&sim);
	for (i = 0; i < BOARD_WIDTH; i++) {
		memcpy(&sim, g, sizeof(struct game));
		headless_drop_tile(i);
		if (sim.score > best_score) {
			best_col = i;
			best_score = sim.score;
		}
	}
	return best_col;
}

struct game *greedy_play_single_game(void) {
    initCharTable();
    struct game *g = init_game(1);
    set_game(g);
    while (!g->game_over) {
        if (g->replace_status == NORMAL
            && g->drop_letter != DROP_BLANK) {
            // normal operation: drop tile into a random column
            headless_drop_tile(get_greedy_best_drop_col(g));
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
