#include "portmanteau.h"
#include "mainMenu.h"
#include "scores.h"
#include "game.h"
#include "words.h"
#include "agents.h"

void initTUI(void) {
	initscr();
	start_color();
	// disable echoing of input to terminal
	noecho();
	cbreak();
	// disable terminal cursor
	curs_set(0);
	// capture function key events (e.g., arrows)
	keypad(stdscr, TRUE);
	init_pair(1, COLOR_RED, COLOR_BLACK);
	init_pair(2, COLOR_GREEN, COLOR_BLACK);
	init_pair(3, COLOR_BLUE, COLOR_BLACK);
	init_pair(4, COLOR_MAGENTA, COLOR_MAGENTA);
	init_pair(5, COLOR_RED, COLOR_RED);
	init_pair(6, COLOR_GREEN, COLOR_GREEN);
	init_pair(7, COLOR_BLUE, COLOR_BLUE);
	init_pair(8, COLOR_WHITE, COLOR_WHITE);
	init_pair(9, COLOR_CYAN, COLOR_CYAN);
	setlocale(LC_ALL, "");
}

int main(int argc, char **argv) {
	initTUI();
	initMenu();
	srand((unsigned int) time(NULL));
	char c;
	const char *selection = NULL;
	while ((c = getch()) != 'q') {
		selection = displayMenu();
		if (strcmp(selection, "Play") == 0) {
			play();
			initMenu();
		} else if (strcmp(selection, "High Scores") == 0) {
			scores();
		} else if (strcmp(selection, "AI: Random Agent") == 0) {
			struct game_state *result = random_play_single_game();
			mvprintw(LINES - 3, 0, "AI score: %d              ", result->stats->score);
			refresh();
		} else if (strcmp(selection, "AI: Sequential Agent") == 0) {
			struct game_state *result = sequential_play_single_game();
			mvprintw(LINES - 3, 0, "AI score: %d              ", result->stats->score);
			refresh();
		} 
	}
	freeMenu();
	refresh();
	endwin();
}
