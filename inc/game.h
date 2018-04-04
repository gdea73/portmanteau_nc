#ifndef GAME_H
#define GAME_H

#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <math.h>
#include <unistd.h>
#include "letter.h"
#include "words.h"
#include "stats.h"
#define BOARD_BLANK '_'
#define BOARD_WIDTH 7
#define BOARD_HEIGHT 7
#define INIT_BOARD_BLANK
#define BOARD_WIN_X 25
#define BOARD_WIN_Y 17
#define PAD_Y 2
#define PAD_X 1
#define DIR_RIGHT 1
#define DIR_LEFT -1
#define DIR_STAY 0
#define MAX_BOARD_WORDS 7 * 5
#define N_RECENT_BREAKS 13
// minimum terminal dimensions to render the game
#define MIN_LINES BOARD_WIN_Y + 2 * PAD_Y
#define MIN_COLS BOARD_WIN_X + 3 * PAD_Y + 7
// 7 == max width of recent words
// #define INIT_BOARD_RANDOM

// delays (pseudo-animations)
#define GRAV_DELAY_MS 20L
#define CHAIN_DELAY_MS 700L
#define REPLACE_DELAY_MS 500L

// line on which messages to the user should be printed
#define MESSAGE_Y PAD_Y - 1
#define BLANK_MESSAGE "Type any capital letter you wish to drop into the board."
#define SELECT_MESSAGE "Select a tile to edit."
#define REPLACE_MESSAGE "Type a new capital letter for the selected tile."
#define GAME_OVER_MESSAGE "GAME OVER!"

// longest possible chain (for the sake of score multipliers)
#define MAX_CHAIN 7

#define BASE_LEVEL_THRESHOLD 10

WINDOW *createWindow(int height, int width, int start_y, int start_x);

int initWindows(void);

void pushRecentBreak(char *word);

void drawRecentBreaks(void);

void drawScore(void);

void draw_message(const char *message);

void drawDropChar(int direction);

int drawBoard(void);

void initBoard(void);

void dropGravity(int dropCol);

void boardGravity(void);

// constants returned by processDrop
#define DROP_SUCCESS 0
#define DROP_COL_FULL 1
#define DROP_GAME_OVER 2

int processDrop(int col);

void breakWords(int chainLevel);

void breakBoardWord(struct boardWord *bw);

char *readBoardWord(struct boardWord *bw);

uint8_t isTileReplacement(void);

void play(void);
#endif
