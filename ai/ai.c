#include "ai.h"
#include "game.h"

static const struct agent agents[] = {{
		get_random_normal_move, get_random_blank_move,
		get_random_replace_move, "random"
	}, {
		get_random_normal_move, get_greedy_blank_move,
		get_random_replace_move, "randgreedyrand"
	}, {
		get_greedy_normal_move, get_random_blank_move,
		get_random_replace_move, "greedy"
	}, {
		get_greedy_normal_move, get_greedy_blank_move,
		get_random_replace_move, "greedy2"
	}, {
		get_column_normal_move, get_random_blank_move,
		get_random_replace_move, "column"
	}
	, {
		get_column_normal_move, get_greedy_blank_move,
		get_random_replace_move, "column_greedyblank"
	}
	, {
		get_sequential_normal_move, get_random_blank_move,
		get_random_replace_move, "sequential"
	}
	, {
		get_sequential_normal_move, get_greedy_blank_move,
		get_random_replace_move, "sequential_greedyblank"
	}

};

void play_AI_game(struct agent agent, struct game *game) {
	struct blank_move blank_move;
	struct replace_move replace_move;
	set_game(game);
	while (!game->game_over) {
		if (game->replace_status == NORMAL) {
			if (game->drop_letter == DROP_BLANK) {
				// BLANK strategy
				blank_move = agent.get_blank_move(game);
				headless_assign_blank(blank_move.letter);
				headless_drop_tile(blank_move.drop_col);
			} else {
				// NORMAL strategy
				headless_drop_tile(agent.get_normal_move(game));
			}
		} else if (game->replace_status == SELECT) {
			// REPLACE strategy
			replace_move = agent.get_replace_move(game);
			headless_replace_tile(replace_move.tile_ID, replace_move.letter);
		} else {
			fprintf(stderr, "replace_status shouldn't be SELECT for AI\n");
		}
	}
}

int main(int argc, char **argv) {
	srand(time(NULL));
	if (argc != 3) {
		fprintf(stderr, "usage: %s n_games strategy_name\n", argv[0]);
		return -1;
	}
	struct agent agent = { 0 };
	int i, total_score, n_games = atoi(argv[1]);
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
		// printf("AI game #%d ended with score: %d\n", i, ai_games[i]->score);
		printf("%d\n", ai_games[i]->score);
	}
	for (i = 0; i < n_games; i++) {
		total_score += ai_games[i]->score;
		free_game(ai_games[i]);
	}
	printf("Average score: %f\n", (float) total_score / n_games);
}
