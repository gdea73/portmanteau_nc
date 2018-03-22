#include "game.h"

char board[BOARD_WIDTH][BOARD_HEIGHT];
char *recentBreaks[N_RECENT_BREAKS];
char dropChar;
int score = 0, selectedCol = 3;
WINDOW *mainwin, *boardwin;

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
	mvwprintw(boardwin, 1, 2, "Score: %d", score);
	wrefresh(boardwin);
}

void drawRecentBreaks(void) {
	int r = BOARD_WIN_Y + 2 * PAD_Y, i;
	mvwprintw(mainwin, r, 2, "Recent Words:");
	for (i = 0; i < N_RECENT_BREAKS; i++) {
		if (recentBreaks[i] == NULL) {
			break;
		}
		wmove(mainwin, r + i + 1, 2);
		wclrtoeol(mainwin);
		wprintw(mainwin, "%s", recentBreaks[i]);
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
	mvwaddch(boardwin, 2, 3 + 3 * selectedCol, dropChar);
	wrefresh(boardwin);
}

int drawBoard(void) {
	int board_pos_x = 3;
	int board_pos_y = 3;
	wmove(boardwin, board_pos_y, board_pos_x);
	for (int c = 0; c < BOARD_WIDTH; c++) {
		for (int r = 0; r < BOARD_HEIGHT; r++) {
			if (mvwaddch(boardwin, board_pos_y, board_pos_x, board[c][r]) == ERR) {
				fprintf(stderr, "Failed to add char to board window at (%d, %d)",
					board_pos_x, board_pos_y);
				return 1;
			}
			board_pos_y += 2;
		}
		board_pos_x += 3;
		board_pos_y = 3;
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
	if (termwidth < 80 || termheight < 23 || (mainwin = initscr()) == NULL) {
		return 1;
	}
	box(mainwin, 0, 0);
	boardwin = subwin(mainwin, BOARD_WIN_Y, BOARD_WIN_X, PAD_Y, 
		(termwidth - BOARD_WIN_X) / 2);
	box(boardwin, 0, 0);
	drawBoard();
	return 0;
}

void initBoard(void) {
	#ifdef INIT_BOARD_BLANK
	// initialize the board with blanks
	for (int c = 0; c < BOARD_HEIGHT; c++) {
		for (int r = 0; r < BOARD_WIDTH; r++) {
			board[c][r] = BLANK; 
		}
	}
	#else
	for (int c = 0; c < BOARD_RAND_HEIGHT; c++) {
		for (int r = 0; r < BOARD_WIDTH; r++) {
			board[c][r] = saneRandChar();
		}
	}
	#endif
}

void dropGravity(int dropCol) {
	for (int r = BOARD_HEIGHT - 1; r >= 0; r--) {
		// traverse the column from base to peak
		if (board[dropCol][r] != BLANK) {
			// replace underlying blanks with the letter
			int r2 = r + 1;
			while (r2 < BOARD_HEIGHT && board[dropCol][r2] == BLANK) {
				board[dropCol][r2] = board[dropCol][r2 - 1];
				// create space in the recently-vacated spots
				board[dropCol][r2 - 1] = BLANK;
				r2++;
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
	if (board[col][0] != BLANK) {
		// the drop is invalid; the selected column is full
		return 1;
	}
	board[col][0] = dropChar;
	dropGravity(col);
	breakWords(1);
	return 0;
}

void breakWords(int chainLevel) {
	struct boardWord *wordsToCheck = calloc(MAX_BOARD_WORDS, sizeof(*wordsToCheck) * MAX_BOARD_WORDS);
	int n_wordsToCheck = 0;
	uint8_t anyWordsBroken = 0;
	int c = 0;
	// read the board for "words," beginning with columns
	for (c = 0; c < BOARD_WIDTH; c++) {
		if (board[c][BOARD_HEIGHT - 1] != BLANK) {
			int startRow = BOARD_HEIGHT - 1;
			while (startRow > 0 && board[c][startRow - 1] != BLANK) {
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
			if (!inWord && board[c][r] != BLANK) {
				inWord = 1;
				wStartCol = c;
			} else if (inWord) {
				if (board[c][r] == BLANK) {
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
			score += wordScore(s) * (int) pow((double) 2, chainLevel - 1);
			// TODO: finalize word-length score multipliers
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
		sleep(0.5); // TODO: animation?
		breakWords(chainLevel + 1); // TODO: finalize chain score multipliers
	}
}

/* Assumes caller has validated that *bw is a valid English word. */
void breakBoardWord(struct boardWord *bw) {
	if (bw->startCol == bw->endCol) {
		for (int r = bw->startRow; r <= bw->endRow; r++) {
			board[bw->endCol][r] = BLANK;
		}
	} else if (bw->startRow == bw->endRow) {
		for (int c = bw->startCol; c <= bw->endCol; c++) {
			board[c][bw->endRow] = BLANK;
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
		s = calloc(endRow - startRow + 1, sizeof(char));
		for (int r = startRow; r <= endRow; r++) {
			s[r - startRow] = board[endCol][r];
		}
	} else if (startRow == endRow) {
		// parse word horizontally
		s = calloc(endCol - startCol + 1, sizeof(char));
		for (int c = startCol; c <= endCol; c++) {
			s[c - startCol] = board[c][endRow];
		}
	}
	return s;
}

void play(void) {
	initBoard();
	if (initWindows() != 0) {
		printf("%s", "There was an error attempting to create the game windows.");
		return;
	}
	int c;
	dropChar = saneRandChar();
	drawDropChar(DIR_RIGHT);
	drawScore();
	drawRecentBreaks();
	while ((c = getch()) != 'q') {
		if (dropChar == '*') {
			if (c > 64 && c < 91) {
				// assign an arbitrary letter to a blank ('*')
				dropChar = c;
				drawDropChar(DIR_STAY);
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
					dropChar = saneRandChar();
					drawBoard();
					drawScore();
					drawRecentBreaks();
					drawDropChar(DIR_STAY);
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
