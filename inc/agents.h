#ifndef AGENTS_H
#define AGENTS_H

struct game_state {
	struct stats *stats;
	char *board;
	uint8_t game_over;
};

struct game_state *random_play_single_game(void);

#endif
