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
#include "agents.h"
#include "game_struct.h"
#define BOARD_BLANK '_'
#define BOARD_WIDTH 7
#define BOARD_HEIGHT 7
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

struct game *get_game(void);

void set_game(struct game *g);

WINDOW *create_window(int height, int width, int start_y, int start_x);

int init_windows(void);

void push_recent_break(char *word);

void draw_recent_breaks(void);

void draw_score(void);

void draw_message(const char *message);

void draw_drop_letter(int direction);

int draw_board(void);

void dropGravity(int dropCol);

void boardGravity(void);

// constants returned by process_drop
#define DROP_SUCCESS 0
#define DROP_COL_FULL 1
#define DROP_GAME_OVER 2

int process_drop(int col);

void break_words(int chainLevel);

void break_board_word(struct boardWord *bw);

char *readBoardWord(struct boardWord *bw);

uint8_t isTileReplacement(void);

struct game *init_game(uint8_t is_headless);

void free_game(struct game *game);

int headless_drop_tile(int drop_col);

int headless_assign_blank(char blank_assignment);

int headless_replace_tile(int tile_ID, char new_letter);

void play(void);
#endif
