/* void breakWords(int dropCol) {
	// TODO: rewrite this prototype such that it takes no arguments,
	// and begins with checking the entire board (exhaustive reading
	// should not be slow enough to merit range-restricted reading).

	// check the drop column for a vertical word,
	// and letters contiguous with the drop for a
	// horizontal word.
	int endRow = BOARD_HEIGHT - 1;
	// determine the endRow; e.g., highest non-blank row
	while (endRow > 0 && board[dropCol][endRow - 1] != BLANK) {
		endRow--;
	}
	struct boardWord *wordsToCheck = malloc(sizeof(*wordsToCheck) * MAX_BOARD_WORDS);
	wordsToCheck[0] = (struct boardWord) {dropCol, BOARD_HEIGHT - 1,
				   					dropCol, endRow};
	int n_boardWords = 1;
	for (int r = BOARD_HEIGHT - 1; r >= endRow; r--) {
		int startCol = dropCol;
		while (startCol > 0 && board[startCol - 1][r] != BLANK) {
			startCol--;
		}
		int endCol = dropCol;
		while (endCol < BOARD_WIDTH && board[startCol + 1][r] != BLANK) {
			endCol++;
		}
		wordsToCheck[n_boardWords++] = (struct boardWord) {startCol, r, endCol, r};
	}
} */
