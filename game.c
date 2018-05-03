#include "game.h"

char *recentBreaks[N_RECENT_BREAKS];
int selectedCol = 3, boardwin_pos_x;
WINDOW *mainwin, *boardwin;
static struct game *game;
const struct timespec gravity_delay = {0, 1000000L * GRAV_DELAY_MS},
                      chain_delay = {0, 1000000L * CHAIN_DELAY_MS},
                      replace_delay = {0, 1000000L * REPLACE_DELAY_MS};

struct game *get_game(void) {
	return game;
}

void set_game(struct game *g) {
	game = g;
}

struct dictionary *getDict(void) {
	static struct dictionary *dict;
	static int initialized = 0;
	if (!initialized) {
		dict = loadDict("./words_scrabble.txt");
		initialized = 1;
	}
	return dict;
}

void push_recent_break(char *word) {
	int i;
	if (strlen(word) > game->longest_word) {
		game->longest_word = strlen(word);
	}
	for (i = N_RECENT_BREAKS - 1; i > 0; i--) {
		strncpy(game->recent_breaks[i], game->recent_breaks[i - 1], 7);
	}
	strncpy(game->recent_breaks[0], word, 7);
}

WINDOW *create_window(int height, int width, int start_y, int start_x) {
	WINDOW *win = newwin(height, width, start_y, start_x);
	box(win, 0, 0);
	wrefresh(win);
	return win;
}

void draw_score(void) {
	int c = 1;
	while (c < BOARD_WIN_X - 1) {
		mvwaddch(boardwin, 1, c, ' ');
		c++;
	}
	int level_width = ceil(logf(game->level));
	mvwprintw(boardwin, 1, 3, "%d", game->score);
	mvwprintw(
		boardwin, 1, BOARD_WIN_X - level_width - 9, "Level %d", game->level
	);
	wrefresh(boardwin);
}

void draw_recent_breaks(void) {
	int recent_breaks_pos_x = boardwin_pos_x - PAD_X - 7;
	mvwprintw(mainwin, PAD_Y + 1, recent_breaks_pos_x, "Recent");
	mvwprintw(mainwin, PAD_Y + 2, recent_breaks_pos_x, "Words:");
	int i, j;
	for (i = 0; i < N_RECENT_BREAKS; i++) {
		if (strnlen(game->recent_breaks[i], 7) == 0) {
			break;
		}
		wmove(mainwin, PAD_Y + 3 + i, recent_breaks_pos_x);
		for (j = 0; j < strlen(game->recent_breaks[i]); j++) {
			waddch(mainwin, game->recent_breaks[i][j]);
		}
		while (j < 7) {
			// erase remnants of previous (longer) words
			waddch(mainwin, ' '); j++;
		}
	}
}

void clear_message(void) {
	int col = 1;
	while (col < COLS - 1) {
		// clear this line, stopping at the border (1 from EOL)
		mvwaddch(mainwin, MESSAGE_Y, col, ' ');
		col++;
	}
}

void draw_message(const char *message) {
	clear_message();
	mvwprintw(mainwin, MESSAGE_Y, (COLS - strlen(message)) / 2, message);
}

void draw_drop_letter(int direction) {
	// overwrite the previous drop character with a space
	mvwaddch(boardwin, 2, 3 + 3 * selectedCol, ' ');
	if (direction == DIR_RIGHT) {
		selectedCol++;
		selectedCol %= BOARD_WIDTH;
	} else if (direction == DIR_LEFT) {
		selectedCol = (selectedCol == 0) ? BOARD_WIDTH - 1 : selectedCol - 1;
	}	
	mvwaddch(boardwin, 2, 3 + 3 * selectedCol, game->drop_letter);
	wrefresh(boardwin);
}

int draw_board(void) {
	int board_x = 3;
	int board_y = 3;
	wmove(boardwin, board_y, board_x);
	int hl_c = -1, hl_r = -1;
	if (game->replace_status != NORMAL) {
		hl_c = game->replace_tile_ID / 7;
		hl_r = game->replace_tile_ID % 7;
	}
	wattron(boardwin, A_BOLD);
	for (int c = 0; c < BOARD_WIDTH; c++) {
		for (int r = 0; r < BOARD_HEIGHT; r++) {
			if (c == hl_c && r == hl_r) {
				// highlight the tile selected for replacement
				wattron(boardwin, A_REVERSE);
			}
			if (mvwaddch(boardwin, board_y, board_x, game->board[c][r]) == ERR) {
				fprintf(
					stderr,
					"Failed to add char to board window at (%d, %d)",
					board_y, board_x
				);
				return 1;
			}
			wattroff(boardwin, A_REVERSE);
			board_y += 2;
		}
		board_x += 3;
		board_y = 3;
	}
	wrefresh(boardwin);
	refresh();
	return 0;
}

int init_windows(void) {
	clear();
	refresh();
	int termwidth = getmaxx(stdscr);
	int termheight = getmaxy(stdscr);
	if (
		termwidth < MIN_COLS || termheight < MIN_LINES
		|| (mainwin = initscr()) == NULL
	) {
		return 1;
	}
	box(mainwin, 0, 0);
	if (termwidth < BOARD_WIN_X + 7 * 2 + 4 * PAD_X) {
		boardwin_pos_x = 3 * PAD_X + 7;
	} else {
		boardwin_pos_x = (termwidth - BOARD_WIN_X) / 2;
	}
	boardwin = subwin(mainwin, BOARD_WIN_Y, BOARD_WIN_X, PAD_Y, boardwin_pos_x);
	box(boardwin, 0, 0);
	draw_board();
	return 0;
}

void dropGravity(int dropCol) {
	for (int r = BOARD_HEIGHT - 1; r >= 0; r--) {
		// traverse the column from base to peak
		if (game->board[dropCol][r] != BOARD_BLANK) {
			// replace underlying blanks with the letter
			int r2 = r + 1;
			while (r2 < BOARD_HEIGHT
				&& game->board[dropCol][r2] == BOARD_BLANK) {
				game->board[dropCol][r2] = game->board[dropCol][r2 - 1];
				// create space in the recently-vacated spots
				game->board[dropCol][r2 - 1] = BOARD_BLANK;
				r2++;
				if (!game->is_headless) {
					if (r != -1 && r2 != BOARD_HEIGHT) {
						nanosleep(&gravity_delay, NULL);
					}
					draw_board();
				}
			}
		}
	}
}

void boardGravity(void) {
	for (int c = 0; c < BOARD_WIDTH; c++) {
		dropGravity(c);
	}
}

int process_drop(int col) {
	if (game->board[col][0] != BOARD_BLANK) {
		// the drop is invalid; the selected column is full
		return DROP_COL_FULL;
	}
	game->board[col][0] = game->drop_letter;
	dropGravity(col);
	break_words(1);
	for (int c = 0; c < BOARD_WIDTH; c++) {
		for (int r = 0; r < BOARD_HEIGHT; r++) {
			if (game->board[c][r] == BOARD_BLANK) {
				return DROP_SUCCESS;
			}
		}
	}
	// game over condition
	return DROP_GAME_OVER;
}

void break_words(int chainLevel) {
	struct boardWord *wordsToCheck = calloc(MAX_BOARD_WORDS, sizeof(*wordsToCheck) * MAX_BOARD_WORDS);
	int n_wordsToCheck = 0;
	uint8_t anyWordsBroken = 0;
	int c = 0;
	if (chainLevel > game->longest_chain) {
		game->longest_chain = chainLevel;
	}
	// read the board for "words," beginning with columns
	for (c = 0; c < BOARD_WIDTH; c++) {
		if (game->board[c][BOARD_HEIGHT - 1] != BOARD_BLANK) {
			int startRow = BOARD_HEIGHT - 1;
			while (startRow > 0 && game->board[c][startRow - 1] != BOARD_BLANK) {
				startRow--;
			}
			wordsToCheck[n_wordsToCheck++] = (struct boardWord) {c,
				startRow, c, BOARD_HEIGHT - 1};
			/* if (n_wordsToCheck > MAX_BOARD_WORDS) {
				fprintf(stderr, "Maximum expected board word count exceeded.");
			} */
		}
	}
	// scan for horizontal (contiguous) "words"
	for (int r = 0; r < BOARD_HEIGHT; r++) {
		c = 0;
		int inWord = 0;
		int wStartCol;
		for (c = 0; c < BOARD_WIDTH; c++) {
			if (!inWord && game->board[c][r] != BOARD_BLANK) {
				inWord = 1;
				wStartCol = c;
			} else if (inWord) {
				if (game->board[c][r] == BOARD_BLANK) {
					// the word has ended; mark its endCol as (c - 1)
					inWord = 0;
					wordsToCheck[n_wordsToCheck++] = (struct boardWord) {
						wStartCol, r, c - 1, r};
				} else if (c == BOARD_WIDTH - 1) {
					// the word intersects the board X-boundary
					inWord = 0;
					wordsToCheck[n_wordsToCheck++] = (struct boardWord) {
						wStartCol, r, c, r};
				}
			}
		}
	}
	// first, extract strings from board coordinates
	char **boardStrings = calloc(n_wordsToCheck, sizeof(char *));
	for (int i = 0; i < n_wordsToCheck; i++) {
		if ((boardStrings[i] = readBoardWord(&wordsToCheck[i])) == NULL) {
			fprintf(stderr, "Reading of string from board coords. failed.");
			return;
		}
	}
	// check if any board "words" are actual English words
	for (int i = 0; i < n_wordsToCheck; i++) {
		int s_valid = 0, s_rev_valid = 0;
		char *s = boardStrings[i];
		char *s_rev = reverse(s);
		s_valid = isValidWord(getDict(), s);
		s_rev_valid = isValidWord(getDict(), s_rev);
		if (s_valid || s_rev_valid) {
			if (s_valid) {
				push_recent_break(s);
			} else {
				push_recent_break(s_rev);
			}
			break_board_word(&wordsToCheck[i]);
			anyWordsBroken = 1;
			game->score += wordScore(s) * chainMultipliers[
				chainLevel < MAX_CHAIN ? chainLevel : MAX_CHAIN
			] * game->level; // linear multiplier per-level
			chainLevel++;
			game->n_words_broken += 1;
			if (game->n_words_broken > game->next_level_threshold) {
				game->next_level_threshold +=
					BASE_LEVEL_THRESHOLD *
					ceil(game->level / 2.0);
				game->level++;
			}
			game->n_tiles_broken += strlen(s);
		}
		free(s);
		free(s_rev);
	}
	free(wordsToCheck);
	free(boardStrings);
	// if any words were broken, enact board gravity, and check for chains.
	if (anyWordsBroken) {
		boardGravity();
		if (!game->is_headless) {
			draw_board();
			if (chainLevel > 2) {
				// delay to let user process words broken
				nanosleep(&chain_delay, NULL);
			}
		}
		break_words(chainLevel);
	}
}

/* Assumes caller has validated that *bw is a valid English word. */
void break_board_word(struct boardWord *bw) {
	struct game *g = game;
	if (bw->startCol == bw->endCol) {
		for (int r = bw->startRow; r <= bw->endRow; r++) {
			g->board[bw->endCol][r] = BOARD_BLANK;
		}
	} else if (bw->startRow == bw->endRow) {
		for (int c = bw->startCol; c <= bw->endCol; c++) {
			g->board[c][bw->endRow] = BOARD_BLANK;
		}
	}
}

/* Attempts to extract a string, oriented vertically or horizontally,
   from the board, using coordinates. Will return a NULL pointer only
   if the coordinates are diagonal or if a BLANK is discovered within
   their bounds. */
char *readBoardWord(struct boardWord *bw) {
	char *s = NULL;
	int startCol = bw->startCol, startRow = bw->startRow;
	int endCol = bw->endCol, endRow = bw->endRow;
	if (startCol == endCol) {
		// parse word vertically
		s = calloc(endRow - startRow + 2, sizeof(char));
		for (int r = startRow; r <= endRow; r++) {
			s[r - startRow] = game->board[endCol][r];
		}
		if (s == NULL) {
			fprintf(stderr, "failed to allocate memory for board word\n");
		}
	} else if (startRow == endRow) {
		// parse word horizontally
		s = calloc(endCol - startCol + 2, sizeof(char));
		for (int c = startCol; c <= endCol; c++) {
			s[c - startCol] = game->board[c][endRow];
		}
		if (s == NULL) {
			fprintf(stderr, "failed to allocate memory for board word\n");
		}
	}
	return s;
}

uint8_t isTileReplacement(void) {
	int tilesOnBoard = 0;
	for (int c = 0; c < BOARD_WIDTH; c++) {
		for (int r = 0; r < BOARD_HEIGHT; r++) {
			if (game->board[c][r] != BOARD_BLANK) tilesOnBoard++;
		}
	}
	if (tilesOnBoard < 20) {
		return 0;
	}
	if (tilesOnBoard > 42) {
		return 1;
	}
	return (int) floor(rand() / (RAND_MAX / 100) < 2 * tilesOnBoard);
}

struct game *init_game(uint8_t is_headless) {
	/* char *board_cols[7];
	for (int i = 0; i < 7; i++) {
		board_cols[i] = malloc(7 * sizeof(char));
		for (int j = 0; j < 7; j++) {
			board_cols[i][j] = BOARD_BLANK;
		}
	} */
	// initialize structs for game statistics and delays
	struct game *game = malloc(sizeof(struct game));
	struct game g = {
		.score = 0,
		.n_moves = 0,
		.level = 1,
		.next_level_threshold = BASE_LEVEL_THRESHOLD,
		.longest_chain = 0,
		.longest_word = 0,
		.n_tiles_broken = 0,
		.n_words_broken = 0,
		.replace_tile_ID = 0,
		.game_over = 0,
		.is_headless = is_headless,
		.replace_status = NORMAL,
	};
	*game = g;
	for (int i = 0; i < N_RECENT_BREAKS; i++) {
		game->recent_breaks[i] = calloc(7 + 1, sizeof(char)); // 7 & nullterm
	}
	for (int c = 0; c < 7; c++) {
		for (int r = 0; r < 7; r++) {
			game->board[c][r] = BOARD_BLANK;
		}
	}
	game->drop_letter = get_next_drop_letter(game);
	generate_tile_set(game->tile_set);
	return game;
}

void free_game(struct game *game) {
	for (int i = 0; i < N_RECENT_BREAKS; i++) {
		free(game->recent_breaks[i]);
	}
	free(game);
}

int headless_drop_tile(int drop_col) {
	if (game->drop_letter == DROP_BLANK) {
		// sanity check: do we have a normal drop tile?
		fprintf(stderr, "The blank tile must be assigned before it is "
				"dropped into the board.");
		return -1;
	}
	if (game->replace_status != NORMAL) {
		// sanity check: are we in normal (as opposed to replacement) mode?
		fprintf(stderr, "A tile cannot be dropped into the board during "
				"a tile replacement operation.");
		return -1;
	}
	int drop_result = process_drop(drop_col);
	if (drop_result == DROP_SUCCESS) {
		game->n_moves++;
		game->drop_letter = get_next_drop_letter(game);
		if (game->drop_letter == DROP_BLANK && isTileReplacement()) {
			game->drop_letter = get_next_drop_letter(game);
			game->replace_status = SELECT;
		}
	} else if (drop_result == DROP_GAME_OVER) {
		game->game_over = 1;
	}
	return drop_result;
}

int headless_assign_blank(char blank_assignment) {
	if (game->drop_letter != DROP_BLANK) {
		fprintf(stderr, "The drop tile is not blank; it cannot be assigned.");
		return -1;
	}
	if (game->replace_status != NORMAL) {
		fprintf(stderr, "A blank cannot be assigned during a tile "
				"replacement operation.");
		return -1;
	}
	if (blank_assignment < 65 || blank_assignment > 90) {
		fprintf(stderr, "The blank assignment '%c' is invalid.",
				blank_assignment);
		return -1;
	}
	game->drop_letter = blank_assignment;
	return 0;
}

int headless_replace_tile(int tile_ID, char new_letter) {
	if (game->replace_status != SELECT) {
		fprintf(stderr, "The replace status must be SELECT (0). "
				"Instead, it is %d.", game->replace_status);
		return -1;
	}
	if (game->board[tile_ID / 7][tile_ID % 7] == BOARD_BLANK) {
		fprintf(stderr, "One can only edit a non-blank tile.");
		return -1;
	}
	if (new_letter < 'A' || new_letter > 'Z') {
		fprintf(stderr, "The blank assignment '%c' is invalid.",
				new_letter);
		return -1;
	}
	game->board[tile_ID / 7][tile_ID % 7] = new_letter;
	game->replace_status = NORMAL;
	break_words(1);
	return 0;
}

void play(void) {
	game = init_game(0);
	if (!game->is_headless) {
		if (init_windows() != 0) {
			printf("%s", "There was an error attempting to create the game windows.");
			return;
		}
		draw_drop_letter(DIR_RIGHT);
		draw_score();
		draw_recent_breaks();
		// special case: game begins with a blank
		if (game->drop_letter == DROP_BLANK) {
			draw_message(BLANK_MESSAGE);
		}
	}
	int c;
	uint8_t quit = 0, drop_result;
	while ((c = getch()) && !quit) {
		if (c == 'q') {
			// TODO: quit confirmation "dialog"
			quit = 1;
		} else if (game->game_over) {
			continue;
		}
		if (game->drop_letter == DROP_BLANK) {
			if (c > 64 && c < 91) {
				// assign an arbitrary (capital) letter to a BLANK
				game->drop_letter = c;
				clear_message();
				draw_drop_letter(DIR_STAY);
			}
			continue;
		} else if (game->replace_status == SELECT) {
			// erase the drop character to avoid confusion
			mvwaddch(boardwin, 2, 3 + 3 * selectedCol, ' ');
			wrefresh(boardwin);
			// arrow keys move selected tile
			switch (c) {
				case KEY_RIGHT:
					// if not already at right border
					if (game->replace_tile_ID < 7 * 6) {
						game->replace_tile_ID += 7;
					}
					break;
				case KEY_LEFT:
					// if not already at left border
					if (game->replace_tile_ID > 6) {
						game->replace_tile_ID -= 7;
					}
					break;
				case KEY_UP:
					if (game->replace_tile_ID % 7 != 0) {
						game->replace_tile_ID--;
					}
					break;
				case KEY_DOWN:
					if (game->replace_tile_ID % 7 != 6) {
						game->replace_tile_ID++;
					}
					break;
				case 10:
					// commit the selection
					if (game->board[game->replace_tile_ID / 7]
							 [game->replace_tile_ID % 7] != BOARD_BLANK) {
						game->replace_status = REPLACE;
						draw_message(REPLACE_MESSAGE);
					}
					break;
				default:
					break;
			}
			draw_board();
			continue;
		} else if (game->replace_status == REPLACE) {
			if (c > 64 && c < 91) {
				game->board[game->replace_tile_ID / 7]
					 [game->replace_tile_ID % 7] = c;
				game->replace_status = NORMAL;
				clear_message();
				// ensure board refresh in case no words are broken
				draw_board();
				draw_drop_letter(DIR_STAY);
				break_words(1);
				draw_score();
				draw_recent_breaks();
			}
			continue;
		} 
		switch (c) {
			case KEY_LEFT:
				draw_drop_letter(DIR_LEFT);
				break;
			case KEY_RIGHT:
				draw_drop_letter(DIR_RIGHT);
				break;
			case 10: /* enter */
				drop_result = process_drop(selectedCol);
				if (drop_result == DROP_SUCCESS) {
					draw_recent_breaks();
					wrefresh(mainwin);
					game->n_moves++;
					game->drop_letter =
						get_next_drop_letter(game);
					if (game->drop_letter == DROP_BLANK) {
						if (isTileReplacement()) {
							game->n_moves++;
							game->drop_letter =
								get_next_drop_letter(game);
							game->replace_status = SELECT;
							draw_message(SELECT_MESSAGE);
						} else {
							draw_message(BLANK_MESSAGE);
						}
					}
					draw_board();
					draw_score();
					draw_drop_letter(DIR_STAY);
				} else if (drop_result == DROP_GAME_OVER) {
					// TODO: game over dialog, save score, etc.
					draw_board();
					draw_message(GAME_OVER_MESSAGE);
					game->game_over = 1;
				}
			break;
			case KEY_RESIZE:
				clear();
				if (COLS < MIN_COLS || LINES < MIN_LINES) {
					mvprintw(0, 0, "Term too small!");
				} else {
					if (init_windows() != 0) {
						printf("%s", "There was an error attempting to create "
									 "the game windows.");
					} else {
						draw_recent_breaks();
						draw_board();
						draw_score();
						draw_drop_letter(DIR_STAY);
					}
				}
				break;
			default: /* we don't care about other keys yet */
				break;
		}
	}
	delwin(boardwin);
	delwin(mainwin);
	freeDict(getDict());
	free_game(game);
	refresh();
	endwin();
}

