#include "game.h"

char board[BOARD_WIDTH][BOARD_HEIGHT];
char *recentBreaks[N_RECENT_BREAKS];
int selectedCol = 3, boardwin_pos_x;
WINDOW *mainwin, *boardwin;
struct stats game_stats;
struct game_state game_state;
uint8_t game_over, is_headless;
const struct timespec gravity_delay = {0, 1000000L * GRAV_DELAY_MS},
                      chain_delay = {0, 1000000L * CHAIN_DELAY_MS},
                      replace_delay = {0, 1000000L * REPLACE_DELAY_MS};

struct dictionary *getDict(void) {
	static struct dictionary *dict;
	static int initialized = 0;
	if (!initialized) {
		dict = loadDict("./words_scrabble.txt");
		initialized = 1;
	}
	return dict;
}

void pushRecentBreak(char *word) {
	int i;
	if (strlen(word) > game_stats.longest_word) {
		game_stats.longest_word = strlen(word);
	}
	for (i = N_RECENT_BREAKS - 1; i > 0; i--) {
		recentBreaks[i] = recentBreaks[i - 1];
	}
	recentBreaks[0] = strdup(word);
}

WINDOW *createWindow(int height, int width, int start_y, int start_x) {
	WINDOW *win = newwin(height, width, start_y, start_x);
	box(win, 0, 0);
	wrefresh(win);

	return win;
}

void drawScore(void) {
	int c = 1;
	while (c < BOARD_WIN_X - 1) {
		mvwaddch(boardwin, 1, c, ' ');
		c++;
	}
	int level_width = ceil(logf(game_stats.level));
	mvwprintw(boardwin, 1, 3, "%d", game_stats.score);
	mvwprintw(
		boardwin, 1, BOARD_WIN_X - level_width - 9,
		"Level %d", game_stats.level
	);
	wrefresh(boardwin);
}

void drawRecentBreaks(void) {
	int recent_breaks_pos_x = boardwin_pos_x - PAD_X - 7;
	mvwprintw(mainwin, PAD_Y + 1, recent_breaks_pos_x, "Recent");
	mvwprintw(mainwin, PAD_Y + 2, recent_breaks_pos_x, "Words:");
	int i, j;
	for (i = 0; i < N_RECENT_BREAKS; i++) {
		if (recentBreaks[i] == NULL) {
			break;
		}
		wmove(mainwin, PAD_Y + 3 + i, recent_breaks_pos_x);
		for (j = 0; j < strlen(recentBreaks[i]); j++) {
			waddch(mainwin, recentBreaks[i][j]);
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

void drawDropChar(int direction) {
	// overwrite the previous drop character with a space
	mvwaddch(boardwin, 2, 3 + 3 * selectedCol, ' ');
	if (direction == DIR_RIGHT) {
		selectedCol++;
		selectedCol %= BOARD_WIDTH;
	} else if (direction == DIR_LEFT) {
		selectedCol = (selectedCol == 0) ? BOARD_WIDTH - 1 : selectedCol - 1;
	}	
	mvwaddch(boardwin, 2, 3 + 3 * selectedCol, game_stats.dropChar);
	wrefresh(boardwin);
}

int drawBoard(void) {
	int board_x = 3;
	int board_y = 3;
	wmove(boardwin, board_y, board_x);
	int hl_c = -1, hl_r = -1;
	if (game_stats.replace_status != NORMAL) {
		hl_c = game_stats.replace_tile_ID / 7;
		hl_r = game_stats.replace_tile_ID % 7;
	}
	wattron(boardwin, A_BOLD);
	for (int c = 0; c < BOARD_WIDTH; c++) {
		for (int r = 0; r < BOARD_HEIGHT; r++) {
			if (c == hl_c && r == hl_r) {
				// highlight the tile selected for replacement
				wattron(boardwin, A_REVERSE);
			}
			if (mvwaddch(boardwin, board_y, board_x, board[c][r]) == ERR) {
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

int initWindows(void) {
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
	drawBoard();
	return 0;
}

void initBoard(void) {
	// initialize the board with blanks
	for (int c = 0; c < BOARD_HEIGHT; c++) {
		for (int r = 0; r < BOARD_WIDTH; r++) {
			board[c][r] = BOARD_BLANK; 
		}
	}
}

void dropGravity(int dropCol) {
	for (int r = BOARD_HEIGHT - 1; r >= 0; r--) {
		// traverse the column from base to peak
		if (board[dropCol][r] != BOARD_BLANK) {
			// replace underlying blanks with the letter
			int r2 = r + 1;
			while (r2 < BOARD_HEIGHT && board[dropCol][r2] == BOARD_BLANK) {
				board[dropCol][r2] = board[dropCol][r2 - 1];
				// create space in the recently-vacated spots
				board[dropCol][r2 - 1] = BOARD_BLANK;
				r2++;
				if (!is_headless) {
					if (r != -1 && r2 != BOARD_HEIGHT) {
						nanosleep(&gravity_delay, NULL);
					}
					drawBoard();
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

int processDrop(int col) {
	if (board[col][0] != BOARD_BLANK) {
		// the drop is invalid; the selected column is full
		return DROP_COL_FULL;
	}
	board[col][0] = game_stats.dropChar;
	dropGravity(col);
	breakWords(1);
	for (int c = 0; c < BOARD_WIDTH; c++) {
		for (int r = 0; r < BOARD_HEIGHT; r++) {
			if (board[c][r] == BOARD_BLANK) {
				return DROP_SUCCESS;
			}
		}
	}
	// game over condition
	return DROP_GAME_OVER;
}

void breakWords(int chainLevel) {
	struct boardWord *wordsToCheck = calloc(MAX_BOARD_WORDS, sizeof(*wordsToCheck) * MAX_BOARD_WORDS);
	int n_wordsToCheck = 0;
	uint8_t anyWordsBroken = 0;
	int c = 0;
	if (chainLevel > game_stats.longest_chain) {
		game_stats.longest_chain = chainLevel;
	}
	// read the board for "words," beginning with columns
	for (c = 0; c < BOARD_WIDTH; c++) {
		if (board[c][BOARD_HEIGHT - 1] != BOARD_BLANK) {
			int startRow = BOARD_HEIGHT - 1;
			while (startRow > 0 && board[c][startRow - 1] != BOARD_BLANK) {
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
			if (!inWord && board[c][r] != BOARD_BLANK) {
				inWord = 1;
				wStartCol = c;
			} else if (inWord) {
				if (board[c][r] == BOARD_BLANK) {
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
				pushRecentBreak(s);
			} else {
				pushRecentBreak(s_rev);
			}
			breakBoardWord(&wordsToCheck[i]);
			anyWordsBroken &= 1;
			game_stats.score += wordScore(s) * chainMultipliers[
				chainLevel < MAX_CHAIN ? chainLevel : MAX_CHAIN
			] * game_stats.level; // linear multiplier per-level
			chainLevel++;
			game_stats.n_words_broken += 1;
			if (game_stats.n_words_broken > game_stats.next_level_threshold) {
				game_stats.next_level_threshold +=
					BASE_LEVEL_THRESHOLD *
					ceil(game_stats.level / 2.0);
				game_stats.level++;
			}
			game_stats.n_tiles_broken += strlen(s);
		}
		free(s);
		free(s_rev);
	}
	free(wordsToCheck);
	free(boardStrings);
	// if any words were broken, enact board gravity, and check for chains.
	if (anyWordsBroken) {
		boardGravity();
		if (!is_headless) {
			drawBoard();
				if (chainLevel > 2) {
					// delay to let user process words broken
					nanosleep(&chain_delay, NULL);
				}
			}
			breakWords(chainLevel);
		}
	}

	/* Assumes caller has validated that *bw is a valid English word. */
	void breakBoardWord(struct boardWord *bw) {
		if (bw->startCol == bw->endCol) {
			for (int r = bw->startRow; r <= bw->endRow; r++) {
				board[bw->endCol][r] = BOARD_BLANK;
			}
		} else if (bw->startRow == bw->endRow) {
			for (int c = bw->startCol; c <= bw->endCol; c++) {
				board[c][bw->endRow] = BOARD_BLANK;
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
				s[r - startRow] = board[endCol][r];
			}
			s[endRow - startRow + 2] = '\0';
		} else if (startRow == endRow) {
			// parse word horizontally
			s = calloc(endCol - startCol + 2, sizeof(char));
			for (int c = startCol; c <= endCol; c++) {
				s[c - startCol] = board[c][endRow];
			}
			s[endCol - startCol + 2] = '\0';
		}
		return s;
	}

	uint8_t isTileReplacement(void) {
		int tilesOnBoard = 0;
		for (int c = 0; c < BOARD_WIDTH; c++) {
			for (int r = 0; r < BOARD_HEIGHT; r++) {
				if (board[c][r] != BOARD_BLANK) tilesOnBoard++;
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

	void init_game(void) {
		initBoard();
		// initialize structs for game statistics and delays
		game_stats = (struct stats) {
			0,						// score
			0,						// number of moves
			1,						// level
			BASE_LEVEL_THRESHOLD,	// number of words broken to level up
			0,						// longest chain
			0,						// longest word broken
			0,						// number of tiles broken
			0,						// number of words broken
			0,						// replacement tile ID
			getNextDropChar(0),		// drop letter
			NORMAL					// tile replacement status
		};
		game_over = 0;
		if (initWindows() != 0) {
			printf("%s", "There was an error attempting to create the game windows.");
			return;
		}
		if (!is_headless) {
			drawDropChar(DIR_RIGHT);
			drawScore();
			drawRecentBreaks();
			// special case: game begins with a blank
			if (game_stats.dropChar == DROP_BLANK) {
				draw_message(BLANK_MESSAGE);
			}
		}
	}

	struct game_state *headless_init_game(void) {
		is_headless = true;
		init_game();
		game_state = (struct game_state) {
			&game_stats,
			board[0],
			0
		};
		return &game_state;
	}

	struct game_state *headless_drop_tile(int drop_col) {
		if (game_stats.dropChar == DROP_BLANK) {
			// sanity check: do we have a normal drop tile?
			fprintf(stderr, "The blank tile must be assigned before it is "
					"dropped into the board.");
			return NULL;
		}
		if (game_stats.replace_status != NORMAL) {
			// sanity check: are we in normal (as opposed to replacement) mode?
			fprintf(stderr, "A tile cannot be dropped into the board during "
					"a tile replacement operation.");
			return NULL;
		}
		uint8_t drop_result = processDrop(selectedCol);
		if (drop_result == DROP_SUCCESS) {
			game_stats.dropChar = getNextDropChar(game_stats.n_moves);
			if (game_stats.dropChar == DROP_BLANK && isTileReplacement()) {
				game_stats.replace_status = SELECT;
			}
		} else if (drop_result == DROP_GAME_OVER) {
			game_state.game_over = 1;
		}
		return &game_state;
	}

	struct game_state *headless_assign_blank(char blank_assignment) {
		if (game_stats.dropChar != DROP_BLANK) {
			fprintf(stderr, "The drop tile is not blank; it cannot be assigned.");
			return NULL;
		}
		if (game_stats.replace_status != NORMAL) {
			fprintf(stderr, "A blank cannot be assigned during a tile "
					"replacement operation.");
			return NULL;
		}
		if (blank_assignment < 65 || blank_assignment > 90) {
			fprintf(stderr, "The blank assignment '%c' is invalid.",
					blank_assignment);
			return NULL;
		}
		game_stats.dropChar = blank_assignment;
		return &game_state;
	}

	struct game_state *headless_replace_tile(int tile_ID, char new_letter) {
		if (game_stats.replace_status != SELECT) {
			fprintf(stderr, "The replace status must be SELECT (0). "
					"Instead, it is %d.", game_stats.replace_status);
			return NULL;
		}
		if (board[tile_ID / 7][tile_ID % 7] == BOARD_BLANK) {
			fprintf(stderr, "One can only edit a non-blank tile.");
			return NULL;
		}
		if (new_letter < 65 || new_letter > 90) {
			fprintf(stderr, "The blank assignment '%c' is invalid.",
					new_letter);
			return NULL;
		}
		board[tile_ID / 7][tile_ID % 7] = new_letter;
		game_stats.replace_status = NORMAL;
		breakWords(1);
		return &game_state;
	}

	void play(void) {
		init_game();
		
		int c;
		uint8_t quit = 0, drop_result;
		while ((c = getch()) && !quit) {
			if (c == 'q') {
				// TODO: quit confirmation "dialog"
				quit = 1;
			} else if (game_over) {
				continue;
			}
			if (game_stats.dropChar == DROP_BLANK) {
				if (c > 64 && c < 91) {
					// assign an arbitrary (capital) letter to a BLANK
					game_stats.dropChar = c;
					clear_message();
					drawDropChar(DIR_STAY);
				}
				continue;
			} else if (game_stats.replace_status == SELECT) {
				// erase the drop character to avoid confusion
				mvwaddch(boardwin, 2, 3 + 3 * selectedCol, ' ');
				wrefresh(boardwin);
				// arrow keys move selected tile
				switch (c) {
					case KEY_RIGHT:
						// if not already at right border
						if (game_stats.replace_tile_ID < 7 * 6) {
							game_stats.replace_tile_ID += 7;
						}
						break;
					case KEY_LEFT:
						// if not already at left border
						if (game_stats.replace_tile_ID > 6) {
							game_stats.replace_tile_ID -= 7;
						}
						break;
					case KEY_UP:
						if (game_stats.replace_tile_ID % 7 != 0) {
							game_stats.replace_tile_ID--;
						}
						break;
					case KEY_DOWN:
						if (game_stats.replace_tile_ID % 7 != 6) {
							game_stats.replace_tile_ID++;
						}
						break;
					case 10:
						// commit the selection
						if (board[game_stats.replace_tile_ID / 7]
								 [game_stats.replace_tile_ID % 7] != BOARD_BLANK) {
							game_stats.replace_status = REPLACE;
							draw_message(REPLACE_MESSAGE);
						}
						break;
					default:
						break;
				}
				drawBoard();
				continue;
			} else if (game_stats.replace_status == REPLACE) {
				if (c > 64 && c < 91) {
					board[game_stats.replace_tile_ID / 7]
						 [game_stats.replace_tile_ID % 7] = c;
					game_stats.replace_status = NORMAL;
					clear_message();
					// ensure board refresh in case no words are broken
					drawBoard();
					drawDropChar(DIR_STAY);
					breakWords(1);
					drawScore();
					drawRecentBreaks();
				}
				continue;
			} 
			switch (c) {
				case KEY_LEFT:
					drawDropChar(DIR_LEFT);
					break;
				case KEY_RIGHT:
					drawDropChar(DIR_RIGHT);
					break;
				case 10: /* enter */
					drop_result = processDrop(selectedCol);
					if (drop_result == DROP_SUCCESS) {
						drawRecentBreaks();
						wrefresh(mainwin);
						game_stats.dropChar = getNextDropChar(game_stats.n_moves);
						if (game_stats.dropChar == DROP_BLANK) {
							if (isTileReplacement()) {
								game_stats.dropChar =
									getNextDropChar(game_stats.n_moves);
								game_stats.replace_status = SELECT;
								draw_message(SELECT_MESSAGE);
							} else {
								draw_message(BLANK_MESSAGE);
							}
						}
						drawBoard();
						drawScore();
						drawDropChar(DIR_STAY);
					} else if (drop_result == DROP_GAME_OVER) {
						// TODO: game over dialog, save score, etc.
						drawBoard();
						draw_message(GAME_OVER_MESSAGE);
					game_over = 1;
				}
				break;
			case KEY_RESIZE:
				clear();
				if (COLS < MIN_COLS || LINES < MIN_LINES) {
					mvprintw(0, 0, "Term too small!");
				} else {
					if (initWindows() != 0) {
						printf("%s", "There was an error attempting to create "
									 "the game windows.");
					} else {
						drawRecentBreaks();
						drawBoard();
						drawScore();
						drawDropChar(DIR_STAY);
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
	refresh();
	endwin();
}
