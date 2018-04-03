#ifndef STATS_H
#define STATS_H

enum replace_status {
	NORMAL,		// normal gameplay; no replacement operation in progress
	SELECT,		// the user is selecting the tile to edit
	REPLACE		// the user is selecting the new letter for the chosen tile
};

struct stats {
	int score;
	int n_moves;
	int level;
	int longest_chain;
	int longest_word;
	int n_tiles_broken;
	int n_words_broken;
	int replace_tile_ID;
	char dropChar;
	enum replace_status replace_status;
};

#endif
