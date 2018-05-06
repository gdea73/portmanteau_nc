#include "ai.h"
#include "game.h"

static void print_game_stats(struct game *game);

static const struct agent agents[] = {{
		get_random_normal_move, get_random_blank_move,
		get_random_replace_move, "random"
	}, {
		get_random_normal_move, get_greedy_blank_move_h1,
		get_random_replace_move, "randgreedyrand"
	}, {
		get_greedy_normal_move_h1, get_random_blank_move,
		get_random_replace_move, "greedy"
	}, {
		get_greedy_normal_move_h1, get_greedy_blank_move_h1,
		get_random_replace_move, "greedy2"
	}, {
		get_column_normal_move, get_random_blank_move,
		get_random_replace_move, "column"
	}, {
		get_column_normal_move, get_greedy_blank_move_h1,
		get_random_replace_move, "column_greedyblank"
	}, {
		get_sequential_normal_move, get_random_blank_move,
		get_random_replace_move, "sequential"
	}, {
		get_sequential_normal_move, get_greedy_blank_move_h1,
		get_random_replace_move, "sequential_greedyblank"
	}, {
		get_greedy_normal_move_h1, get_greedy_blank_move_h1,
		get_greedy_replace_move_h1, "gh1"
	}, {
		get_greedy_normal_move_h1, get_random_blank_move,
		get_random_replace_move, "gh1randrand"
	}, {
		get_greedy_normal_move_h2, get_random_blank_move,
		get_random_replace_move, "gh2randrand"
	}, {
		get_greedy_normal_move_h2, get_greedy_blank_move_h2,
		get_greedy_replace_move_h2, "gh2"
	}, {
		get_greedy_normal_move_h3, get_greedy_blank_move_h1,
		get_greedy_replace_move_h1, "gh3gh1gh1"
	}, {
		get_greedy_normal_move_h3, get_greedy_blank_move_h3,
		get_greedy_replace_move_h3, "gh3"
	}
};

void play_AI_game(struct agent agent, struct game *game) {
	struct blank_move blank_move;
	struct replace_move replace_move;
	int normal_drop_col;
	uint8_t print_game = (verbosity_level > 1 && n_games < 10);
	set_game(game);
	while (!game->game_over) {
		if (print_game) {
			printf("score: %d; letter: %c\n", game->score, game->drop_letter);
			print_board(game->board);
		}
		if (game->replace_status == NORMAL) {
			if (game->drop_letter == DROP_BLANK) {
				// BLANK strategy
				blank_move = agent.get_blank_move(game);
				if (print_game) {
					printf("(blank) move: %c to col %d\n", blank_move.letter,
						blank_move.drop_col);
				}
				headless_assign_blank(blank_move.letter);
				headless_drop_tile(blank_move.drop_col);
			} else {
				// NORMAL strategy
				normal_drop_col = agent.get_normal_move(game);
				if (print_game) {
					printf("(normal) move: %d\n", normal_drop_col);
				}
				headless_drop_tile(normal_drop_col);
			}
		} else if (game->replace_status == SELECT) {
			// REPLACE strategy
			replace_move = agent.get_replace_move(game);
			if (print_game) {
				printf("(replace) move: %c to ID %d\n", replace_move.letter,
					replace_move.tile_ID);
			}
			headless_replace_tile(replace_move.tile_ID, replace_move.letter);
		} else {
			fprintf(stderr, "replace_status shouldn't be SELECT for AI\n");
		}
	}
}

static void print_game_stats(struct game *game) {
	printf("score: %d\n", game->score);
	printf("n_moves: %d\n", game->n_moves);
	printf("level: %d\n", game->level);
	printf("longest_chain: %d\n", game->longest_chain);
	printf("longest_word: %d\n", game->longest_word);
	printf("n_tiles_broken: %d\n", game->n_tiles_broken);
	printf("n_words_broken: %d\n", game->n_words_broken);
	printf("recent breaks:\n");
	for (int i = 0; i < N_RECENT_BREAKS && game->recent_breaks[i]; i++) {
		printf("\t%s\n", game->recent_breaks[i]);
	}
	print_board(game->board);
}

int main(int argc, char **argv) {
	srand(time(NULL));
	if (argc < 3 || argc > 4) {
		fprintf(stderr, "usage: %s n_games strategy_name [-v[v[v]]]\n", argv[0]);
		return -1;
	}
	verbosity_level = 0;
	if (argc == 4) {
		while (verbosity_level < 3
		       && strncmp("-vvv", argv[3], verbosity_level + 2) == 0) {
			verbosity_level++;
		}
	}
	printf("verbosity level: %d\n", verbosity_level);
	struct agent agent = { 0 };
	int i, total_score = 0, best_score = 0;
	n_games = atoi(argv[1]);
	float average_score = 0.0f, std_dev = 0.0f;
	struct game **ai_games = malloc(n_games * sizeof(struct game *));
	for (i = 0; i < ARRAY_SIZE(agents); i++) {
		if (strncmp(argv[2], agents[i].strategy_name,
			STRATEGY_NAME_MAX_LENGTH) == 0) {
			agent = agents[i];
			break;
		}
	}
	if (!agent.get_normal_move) {
		fprintf(stderr, "agent/strategy %s could not be found.\n", argv[2]);
		return -1;
	}
	printf("The %s agent will play %d successive games.\n", argv[2], n_games);
	for (i = 0; i < n_games; i++) {
		ai_games[i] = init_game(1); // all AI games are headless
		play_AI_game(agent, ai_games[i]);
		if (verbosity_level > 0) {	
			printf("%d\n", ai_games[i]->score);
		}
	}
	if (n_games < 10 && verbosity_level > 0) {
		for (i = 0; i < n_games; i++) {
			print_game_stats(ai_games[i]);
		}
	}
	for (i = 0; i < n_games; i++) {
		if (ai_games[i]->score > best_score) {
			best_score = ai_games[i]->score;
		}
		total_score += ai_games[i]->score;
	}
	average_score = total_score / (float) n_games;
	// calculate the standard deviation
	for (i = 0; i < n_games; i++) {
		std_dev += pow(ai_games[i]->score - average_score, 2);
		free_game(ai_games[i]);
	}
	std_dev = sqrt(std_dev / (float) n_games);
	printf("Average score: %f\n", average_score);
	printf("Standard deviation: %f\n", std_dev);
	printf("Best score: %d\n", best_score);
	// cleanup
	free(ai_games);
	if (mdict != NULL) {
		freeDict(mdict);
	}
	freeDict(getDict());
}
