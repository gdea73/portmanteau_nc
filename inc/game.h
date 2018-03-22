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
#define BLANK '_'
#define BOARD_WIDTH 7
#define BOARD_HEIGHT 7
#define INIT_BOARD_BLANK
#define BOARD_WIN_X 25
#define BOARD_WIN_Y 17
#define PAD_Y 2
#define DIR_RIGHT 1
#define DIR_LEFT -1
#define DIR_STAY 0
#define MAX_BOARD_WORDS 7 * 5 // TODO: redefine this in terms of BOARD_WIDTH
#define N_RECENT_BREAKS 10
// #define INIT_BOARD_RANDOM

WINDOW *createWindow(int height, int width, int start_y, int start_x);

int initWindows(void);

void pushRecentBreak(char *word);

void drawRecentBreaks(void);

void drawScore(void);

void drawDropChar(int direction);

int drawBoard(void);

void initBoard(void);

void dropGravity(int dropCol);

void boardGravity(void);

int processDrop(int col);

void breakWords(int chainLevel);

void breakBoardWord(struct boardWord *bw);

char *readBoardWord(struct boardWord *bw);

char saneRandChar(void);

void play(void);
#endif
