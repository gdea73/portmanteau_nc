#include "game.h"

char board[BOARD_WIDTH][BOARD_HEIGHT];
char *recentBreaks[N_RECENT_BREAKS];
int selectedCol = 3, boardwin_pos_x;
WINDOW *mainwin, *boardwin;
struct stats game_stats;
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
	// mvwprintw(mainwin, 1, 1, "adding recent break '%s'", word);
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
	mvwprintw(boardwin, 1, 2, "Score: %d", game_stats.score);
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
	if (game_stats.replace_status == SELECT) {
		hl_c = game_stats.replace_tile_ID / 7;
		hl_r = game_stats.replace_tile_ID % 7;
	}
	for (int c = 0; c < BOARD_WIDTH; c++) {
		for (int r = 0; r < BOARD_HEIGHT; r++) {
			if (c == hl_c && r == hl_r) {
				// highlight the tile selected for replacement
				attron(COLOR_PAIR(3));
			}
			if (mvwaddch(boardwin, board_y, board_x, board[c][r]) == ERR) {
				fprintf(
					stderr,
					"Failed to add char to board window at (%d, %d)",
					board_y, board_x
				);
				return 1;
			}
			attroff(COLOR_PAIR(3));
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
	#ifdef INIT_BOARD_BLANK
	// initialize the board with blanks
	for (int c = 0; c < BOARD_HEIGHT; c++) {
		for (int r = 0; r < BOARD_WIDTH; r++) {
			board[c][r] = BOARD_BLANK; 
		}
	}
	#else
	for (int c = 0; c < BOARD_RAND_HEIGHT; c++) {
		for (int r = 0; r < BOARD_WIDTH; r++) {
			board[c][r] = getNextDropChar(game_stats.n_moves);
		}
	}
	#endif
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
				nanosleep(&gravity_delay, NULL);
				drawBoard();
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
		return 1;
	}
	board[col][0] = game_stats.dropChar;
	dropGravity(col);
	breakWords(1);
	return 0;
}

void breakWords(int chainLevel) {
	struct boardWord *wordsToCheck = calloc(MAX_BOARD_WORDS, sizeof(*wordsToCheck) * MAX_BOARD_WORDS);
	int n_wordsToCheck = 0;
	uint8_t anyWordsBroken = 0;
	int c = 0;
	if (chainLevel > 1) {
		nanosleep(&chain_delay, NULL); // delay to let user process words broken
	}
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
			anyWordsBroken = 1;
			game_stats.score += wordScore(s) * chainMultipliers[
				chainLevel < MAX_CHAIN ? chainLevel : MAX_CHAIN
			] * game_stats.level; // linear multiplier per-level
			chainLevel++;
			game_stats.n_words_broken += 1;
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
		drawBoard();
		breakWords(chainLevel + 1);
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
	return (rand() / (RAND_MAX / 100) < 2 * tilesOnBoard);
}

void play(void) {
	initBoard();
	// initialize structs for game statistics and delays
	game_stats = (struct stats) {
		0,					// score
		0,					// number of moves
		1,					// level
		0,					// longest chain
		0,					// longest word broken
		0,					// number of words broken
		0,					// replacement tile ID
		getNextDropChar(0),	// drop letter
		NORMAL				// tile replacement status
	};
	if (initWindows() != 0) {
		printf("%s", "There was an error attempting to create the game windows.");
		return;
	}
	int c;
	game_stats.dropChar = getNextDropChar(game_stats.n_moves);
	drawDropChar(DIR_RIGHT);
	drawScore();
	drawRecentBreaks();
	while ((c = getch())) {
		if (c == 'q') {
			// TODO: quit confirmation "dialog"
			break;
		}
		if (game_stats.dropChar == DROP_BLANK) {
			if (isTileReplacement()) {
				game_stats.replace_status = SELECT;
			} else if (c > 64 && c < 91) {
				// assign an arbitrary (capital) letter to a BLANK
				game_stats.dropChar = c;
				drawDropChar(DIR_STAY);
			}
			continue;
		} else if (game_stats.replace_status == SELECT) {
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
			}
			continue;
		} else if (game_stats.replace_status == REPLACE) {
			if (c > 64 && c < 91) {
				board[game_stats.replace_tile_ID / 7]
				     [game_stats.replace_tile_ID % 7] = c;
				game_stats.replace_status = NORMAL;
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
				if (processDrop(selectedCol) == 0) {
					drawRecentBreaks();
					game_stats.dropChar = getNextDropChar(game_stats.n_moves);
					drawBoard();
					drawScore();
					drawDropChar(DIR_STAY);
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
}
