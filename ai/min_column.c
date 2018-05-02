#include "ai.h"
#include "game.h"

int get_column_normal_move(struct game *game) {
	int column_height[14] = {0};
	for (int i = 0; i < BOARD_WIDTH; i++) {
		for (int j = 0; j < BOARD_HEIGHT; j++) {
			if (game->board[i][j] != BOARD_BLANK) {
				column_height[i]+=1;
			}
		}
	}
	// identify columns with fewest letters
	int shortest = 7;
	for (int i = 0; i < 7; i++) {
		if (column_height[i] <= shortest) {
			shortest = column_height[i];
		}
	}
	// indicate the columns
	for (int i = 0; i < 7; i++) {
		if (column_height[i] == shortest) {
			column_height[i+7] = 1;
		}
	}
	// pick randomly from columns containing fewest letters
	int column_choice = rand_int(6);
	int column_check = 0;
	while (column_check == 0) {
		if (column_height[column_choice+7] == 1) {
			column_check = 1;
		}
		column_choice = rand_int(6);
	}
	return column_choice;
}
