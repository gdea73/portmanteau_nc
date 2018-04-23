#ifndef AGENTS_H
#define AGENTS_H

#include "portmanteau.h"
#include "utils.h"
#include "game.h"

struct game_state {
	struct stats *stats;
	char *board;
	uint8_t game_over;
};

struct game_state *random_play_single_game(void);
struct game_state *sequential_play_single_game(void);

#endif
