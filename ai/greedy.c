#include "ai.h"
#include "game.h"

#define MAX_PROSPECTIVE_WORDS 200
#define MIRRORED_DICT_FILE "./words_mirrored.txt"

// constants regarding heuristic function h2() (and h3())
#define COL_SCORE_LO -100
#define COL_SCORE_HI 100
#define WORD_SCORE_HI 2500
#define COL_SCORE_SHORT 70

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

struct dictionary *mdict = NULL;

static int h1(struct game *g);

static void get_prospective_words(char *substring, char **output);
static int index_of_substring(struct dictionary *mdict, char *substring);
static int bin_substring_search(
	char **words, char *substring, size_t start_idx, size_t end_idx
);
// static uint8_t substring_compare(char *word, char *substring);

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

static int index_of_substring(struct dictionary *mdict, char *substring) {
	return bin_substring_search(mdict->words, substring, 0, mdict->length - 1);
}

static int bin_substring_search(
	char **words, char *substring, size_t start_idx, size_t end_idx
) {
	size_t l = strnlen(substring, 7 - 1);
	if (end_idx - start_idx <= 1) {
		if (strncmp(words[start_idx], substring, l) == 0) {
			return start_idx;
		}
		if (strncmp(words[end_idx], substring, l) == 0) {
			return end_idx;
		}
		return -1; // not found
	}
	size_t m_idx = (start_idx + end_idx) / 2;
	char *m = words[m_idx];
	if (strncmp(substring, m, l) > 0) {
		return bin_substring_search(words, substring, m_idx, end_idx);
	} else if (strncmp(substring, m, l) < 0) {
		return bin_substring_search(words, substring, start_idx, m_idx);
	}
	return m_idx;
}

/* static uint8_t substring_compare(char *word, char *substring) {
	int l = 0;
	// printf("comparing word %s and substring %s\n", word, substring);
	while (word && substring && l < 7) {
		if (*word != *substring) {
			return 0;
		}
		l++; word++; substring++;
	}
	return 1;
} */

static void get_prospective_words(char *substring, char **output) {
	// printf("getting prospective words for substring %s\n", substring);
	int match_idx, i;
	size_t match_count = 0, l;
	l = strnlen(substring, 7 - 1);
	if (mdict == NULL) {
		mdict = loadDict(MIRRORED_DICT_FILE);
	}
	// first, look for any single occurrence of the substring
	match_idx = index_of_substring(mdict, substring);
	if (match_idx == -1) {
		return;
	}
	output[match_count++] = mdict->words[match_idx];
	// search forwards for substring matches
	i = match_idx + 1;
	while (
		i < mdict->length
		&& match_count <= MAX_PROSPECTIVE_WORDS
		&& strncmp(mdict->words[i], substring, l) == 0
	) {
		output[match_count++] = mdict->words[i++];
	}
	// also search backwards for additional substring matches
	i = match_idx - 1;
	// printf("i %d\n", i);
	// printf("words[i] %s\n", mdict->words[i]);
	while (
		i >= 0
		&& match_count <= MAX_PROSPECTIVE_WORDS
		&& strncmp(mdict->words[i], substring, l) == 0
	) {
		output[match_count++] = mdict->words[i--];
	}
}

// h1(game): game->score
static int h1(struct game *g) {
	return g->score;
}

int get_greedy_normal_move_h1(struct game *g) {
	return get_greedy_normal_move(g, h1);
}

static void print_board(char board[7][7]) {
	int c, r;
	for (r = 0; r < 7; r++) {
		for (c = 0; c < 7; c++) {
			printf("%-5c", board[c][r]);
		}
		printf("\n");
	}
}

// h2(game): "column scoring" method
static int h2(struct game *g) {
	int score = 0, c;
	for (c = 0; c < 7; c++) {
		int col_score = h2_col_score(g, c);
		if (verbosity_level > 1) {
			// print every column's score
			printf("%-5d", col_score);
		}
		score += col_score;
	}
	if (verbosity_level > 1) {
		printf("\n");
		print_board(g->board);
		printf("board \"score\": %d\n\n", score);
	}
	return score;
}

int get_greedy_normal_move_h2(struct game *g) {
	return get_greedy_normal_move(g, h2);
}

static int h2_col_score(struct game *g, int col) {
	static char *prospective_words[MAX_PROSPECTIVE_WORDS] = { 0 };
	static struct boardWord bword;
	char *board_word_bw, *board_word;
	int col_score = 0, height = 0, i = 0, total = 0;
	while (height < 7 && g->board[col][7 - 1 - height] != BOARD_BLANK) {
		height++;
	}
	// printf("looking at col %d: %c%c%c%c%c%c%c\n", col, g->board[col][0], g->board[col][1], g->board[col][2], g->board[col][3], g->board[col][4], g->board[col][5], g->board[col][6]);
	// printf("height: %d\n", height);
	if (height == 7) {
		col_score = COL_SCORE_LO;
	} else if (height <= 1) {
		col_score = COL_SCORE_SHORT;
	} else {
		memset(prospective_words, 0, MAX_PROSPECTIVE_WORDS * sizeof(char *));
		bword = (struct boardWord) { col, 7 - height, col, 7 - 1 };
		board_word_bw = readBoardWord(&bword);
		board_word = reverse(board_word_bw);
		// printf("\nboard word %d: %s\n", col, board_word);
		get_prospective_words(board_word, prospective_words);
		if (prospective_words[0] == NULL) {
			// no vertical words are possible in this column
			col_score = COL_SCORE_LO * (height / 7.0f);
		} else {
			i = 0;
			while (prospective_words[i] != NULL && i < MAX_PROSPECTIVE_WORDS) {
				// calculate the average score of prospective words
				total += wordScore(prospective_words[i]);
				i++;
			}
			// scale the average according to maximum word score
			int average_score = (total / (float) i);
			col_score = average_score * (COL_SCORE_HI / (float) WORD_SCORE_HI);
			// scale it according to column height
			// col_score *= height / 6.0f;
		}
		free(board_word);
		free(board_word_bw);
	}
	return col_score;
}

int get_greedy_normal_move_h3(struct game *g) {
	static struct game sim_h1;
	sim_h1 = *g;
	int best_h1_col = get_greedy_normal_move_h1(g);
	set_game(&sim_h1);
	headless_drop_tile(best_h1_col);
	set_game(g);
	if (sim_h1.score > g->score) {
		// h1's best move breaks at least one word
		if (verbosity_level > 1) printf("h1 had best score: %d\n", sim_h1.score);
		return best_h1_col;
	}
	// all moves are equal w.r.t. h1 score; use h2
	if (verbosity_level > 1) printf("fallback to h2\n");
	return get_greedy_normal_move_h2(g);
}

static int get_greedy_normal_move(
	struct game *g, int (*heuristic)(struct game *g)
) {
	static struct game sim;
	int best_col, i, best_score = 0, h;
	best_col = rand_int(6);
	set_game(&sim);
	for (i = 0; i < 7; i++) {
		if (g->board[i][0] == BOARD_BLANK) {
			// memcpy(&sim, g, sizeof(struct game));
			sim = *g;
			headless_drop_tile(i);
			h = heuristic(&sim);
			if (verbosity_level > 1) printf("col %d sim score: %d\n", i, h);
			if (h > best_score) {
				if (verbosity_level > 1) printf("new best score: %d\n", h);
				best_col = i;
				best_score = h;
			}
		}
	}
	set_game(g);
	if (verbosity_level > 1) {
		printf("best column: %d; best score: %d\n", best_col, best_score);
	}
	return best_col;
}

struct blank_move get_greedy_blank_move_h1(struct game *g) {
	return get_greedy_blank_move(g, h1);
}

struct blank_move get_greedy_blank_move_h2(struct game *g) {
	return get_greedy_blank_move(g, h2);
}

struct blank_move get_greedy_blank_move_h3(struct game *g) {
	static struct game sim_h1;
	struct blank_move move_h1 = get_greedy_blank_move_h1(g);
	set_game(&sim_h1);
	headless_assign_blank(move_h1.letter);
	headless_drop_tile(move_h1.drop_col);
	set_game(g);
	if (sim_h1.score > g->score) {
		// h1's best move breaks at least one word
		if (verbosity_level > 1) printf("h1 had best score: %d\n", sim_h1.score);
		return move_h1;
	}
	// all moves are equal w.r.t. h1 score; use h2
	if (verbosity_level > 1) printf("fallback to h2\n");
	return get_greedy_blank_move_h2(g);
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

struct replace_move get_greedy_replace_move_h2(struct game *g) {
	return get_greedy_replace_move(g, h2);
}

struct replace_move get_greedy_replace_move_h3(struct game *g) {
	static struct game sim_h1;
	struct replace_move move_h1 = get_greedy_replace_move_h1(g);
	set_game(&sim_h1);
	headless_replace_tile(move_h1.tile_ID, move_h1.letter);
	set_game(g);
	if (sim_h1.score > g->score) {
		// h1's best move breaks at least one word
		if (verbosity_level > 1) printf("h1 had best score: %d\n", sim_h1.score);
		return move_h1;
	}
	// all moves are equal w.r.t. h1 score; use h2
	if (verbosity_level > 1) printf("fallback to h2\n");
	return get_greedy_replace_move_h2(g);
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
	if (verbosity_level > 1) {
		printf(
			"replacement: %d from %c to %c\n", best_move.tile_ID,
			g->board[best_move.tile_ID / 7][best_move.tile_ID % 7],
			best_move.letter
		);
	}
	return best_move;
}
