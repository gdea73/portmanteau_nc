#ifndef GAME_STRUCT_H
#define GAME_STRUCT_H

#include "portmanteau.h"

enum replace_status {
	NORMAL,		// normal gameplay; no replacement operation in progress
	SELECT,		// the user is selecting the tile to edit
	REPLACE		// the user is selecting the new letter for the chosen tile
};

#define BOARD_WIDTH 7
#define BOARD_HEIGHT 7
#define CHAR_TABLE_SIZE 100
#define N_RECENT_BREAKS 13

struct game {
	int score;
	int n_moves;
	int level;
	int next_level_threshold;
	int longest_chain;
	int longest_word;
	int n_tiles_broken;
	int n_words_broken;
	int replace_tile_ID;
	uint8_t game_over;
	uint8_t is_headless;
	char drop_letter;
	enum replace_status replace_status;
	char board[BOARD_WIDTH][BOARD_HEIGHT];
	char tile_set[CHAR_TABLE_SIZE];
	char *recent_breaks[N_RECENT_BREAKS];
};

#endif
