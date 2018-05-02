#include "ai.h"
#include "game.h"
#include "words.h"

#define MAX_PROSPECTIVE_WORDS 100
#define MIRRORED_DICT_FILE "../words_mirrored.txt"

// constants regarding heuristic function h2()
#define COL_SCORE_MIN -100
#define COL_SCORE_MAX 100
#define WORD_SCORE_MAX 7424 // "MUZIJKS" is a top-tier power play
// TODO: determine a good value for this
#define BASE_SCALE 0.1 

// these were precalculated by counting occurrences in words_mirrored.txt;
// e.g., :%s/^A.*$//gn counts all words beginning or ending with 'A'.
static int letter_freqs[] = {
	4685,	// A  
	3929, 	// B
	5189, 	// C
	8217, 	// D
	7522, 	// E
	2728, 	// F
	4777, 	// G
	3166, 	// H
	1582, 	// I
	789, 	// J
	1803, 	// K
	4083, 	// L
	3688, 	// M
	3625, 	// N
	2129, 	// O
	4448, 	// P
	296, 	// Q
	7493, 	// R
	22059,	// S
	6250, 	// T
	1259, 	// U
	951, 	// V
	1994, 	// W
	309, 	// X
	4225, 	// Y
	406, 	// Z
};

static int h1(struct game *g);

static void get_prospective_words(char *substring, char **output);
static size_t index_of_substring(struct dictionary *dict, char *substring);
static size_t bin_substring_search(
	char **words, char *substring, size_t start_idx, size_t end_idx
);
static uint8_t substring_compare(char *word, char *substring);

static int h2(struct game *g);
static int h2_col_score(struct game *g, int col);

static int get_greedy_normal_move(
	struct game *g, int (*heuristic)(struct game *g)
);
static struct blank_move get_greedy_blank_move(
	struct game *g, int (*heuristic)(struct game *g)
);
static struct replace_move get_greedy_replace_move(
	struct game *g, int (*heuristic)(struct game *g)
);

// h1(game): game->score
static int h1(struct game *g) {
	return g->score;
}
int get_greedy_normal_move_h1(struct game *g) {
	return get_greedy_normal_move(g, h1);
}

static size_t index_of_substring(struct dictionary *dict, char *substring) {
	return bin_substring_search(dict->words, substring, 0, dict->length - 1);
}

static size_t bin_substring_search(
	char **words, char *substring, size_t start_idx, size_t end_idx
) {
	if (end_idx - start_idx <= 1) {
		if (substring_compare(words[end_idx], substring)) {
			return end_idx;
		}
		if (substring_compare(words[start_idx], substring)) {
			return start_idx;
		}
		return -1; // not found
	}
	size_t m_idx = (start_idx + end_idx) / 2;
	char *m = words[m_idx];
	if (strncmp(substring, m, 7) > 0) {
		return bin_substring_search(words, substring, m_idx + 1, end_idx);
	} else if (strncmp(substring, m, 7) < 0) {
		return bin_substring_search(words, substring, start_idx, m_idx - 1);
	}
	return m_idx;
}

static uint8_t substring_compare(char *word, char *substring) {
	int l = 0;
	while (word && substring && l < 7) {
		if (*word != *substring) {
			return 0;
		}
		l++; word++; substring++;
	}
	return 1;
}

static void get_prospective_words(char *substring, char **output) {
	static struct dictionary *dict = NULL;
	size_t match_idx, match_count, i;
	if (dict == NULL) {
		dict = loadDict(MIRRORED_DICT_FILE);
	}
	// first, look for any single occurrence of the substring
	match_idx = index_of_substring(dict, substring) == -1;
	if (match_idx == -1) {
		return;
	}
	match_count = 1;
	i = match_idx;
	while (
		match_count <= MAX_PROSPECTIVE_WORDS
		&& substring_compare(dict->words[i], substring)
		&& i < dict->length
	) {
		output[match_count++ - 1] = dict->words[i++];
	}
	// also search backwards for additional substring matches
	i = match_idx - 1;
	while (
		match_count <= MAX_PROSPECTIVE_WORDS
		&& substring_compare(dict->words[i], substring)
		&& i >= 0
	) {
		output[match_count++ - 1] = dict->words[i--];
	}
}

// h2(game): "column scoring" method
static int h2(struct game *g) {
	int score = 0, c;
	for (c = 0; c < 7; c++) {
		score += h2_col_score(g, c);
	}
	return score;
}

static int h2_col_score(struct game *g, int col) {
	static char *prospective_words[MAX_PROSPECTIVE_WORDS] = { 0 };
	int col_score = 0, height = 0, i = 0, total = 0;
	while (g->board[col][7 - 1 - height] != BOARD_BLANK) {
		height++;
	}
	if (height == 7) {
		col_score = COL_SCORE_MIN;
	} else if (height == 1) {
		col_score = letter_freqs[g->board[col][7 - 1 - height] - 'A']
			* (COL_SCORE_MAX / (float) letter_freqs['S' - 'A']) * BASE_SCALE;
	} else {
		char *board_word = readBoardWord(
			&((struct boardWord) { col, 7 - 1, col, 7 - 1 - height })
		);
		get_prospective_words(board_word, prospective_words);
		if (prospective_words[0] == NULL) {
			// no vertical words are possible in this column
			col_score = COL_SCORE_MIN * (height / 7.0f);
		} else {
			i = 0;
			while (prospective_words[i] != NULL) {
				// calculate the average score of prospective words
				total += wordScore(prospective_words[i]);
				i++;
			}
			// scale the average
			col_score = (total / (float) i) * (COL_SCORE_MAX / WORD_SCORE_MAX);
		}
		free(board_word);
	}
	return col_score;
}

static int get_greedy_normal_move(
	struct game *g, int (*heuristic)(struct game *g)
) {
	static struct game sim;
	int best_col, i, best_score = g->score;
	best_col = rand_int(7);
	set_game(&sim);
	for (i = 0; i < 7; i++) {
		if (g->board[i][0] == BOARD_BLANK) {
			// memcpy(&sim, g, sizeof(struct game));
			sim = *g;
			headless_drop_tile(i);
			if (sim.score > best_score) {
				best_col = i;
				best_score = sim.score;
			}
		}
	}
	set_game(g);
	return best_col;
}

struct blank_move get_greedy_blank_move_h1(struct game *g) {
	return get_greedy_blank_move(g, h1);
}

static struct blank_move get_greedy_blank_move(
	struct game *g, int (*heuristic)(struct game *g)
) {
	static struct game sim;
	char c;
	int i, best_score = g->score;
	 // default move in case all equal
	struct blank_move best_move = get_random_blank_move(g);
	set_game(&sim);
	for (i = 0; i < 7; i++) {
		if (g->board[i][0] == BOARD_BLANK) {
			for (c = 'A'; c <= 'Z'; c++) {
				// memcpy(&sim, g, sizeof(struct game));
				sim = *g;
				headless_assign_blank(c);
				headless_drop_tile(i);
				if (sim.score > best_score) {
					best_score = sim.score;
					best_move.letter = c;
					best_move.drop_col = i;
				}
			}
		}
	}
	// done simulating: restore normal game state
	set_game(g);
	return best_move;
}

struct replace_move get_greedy_replace_move_h1(struct game *g) {
	return get_greedy_replace_move(g, h1);
}

static struct replace_move get_greedy_replace_move(
	struct game *g, int (*heuristic)(struct game *g)
) {
	static struct game sim;
	char c;
	int tile_ID, best_score = g->score;
	// default to a random replacement in case no option breaks a word
	struct replace_move best_move = get_random_replace_move(g);
	set_game(&sim);
	for (tile_ID = 0; tile_ID < 7 * 7; tile_ID++) {
		if (g->board[tile_ID / 7][tile_ID % 7] != BOARD_BLANK) {
			for (c = 'A'; c <= 'Z'; c++) {
				// memcpy(&sim, g, sizeof(struct game));
				sim = *g;
				headless_replace_tile(tile_ID, c);
				if (sim.score > best_score) {
					best_move.tile_ID = tile_ID;
					best_move.letter = c;
				}
			}
		}
	}
	// done simulating: restore normal game state
	set_game(g);
	if (best_score > g->score) {
		printf(
			"replacement score: %d (increase %d)\n", best_score,
			best_score - g->score
		);
	}
	return best_move;
}
