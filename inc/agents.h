#ifndef AGENTS_H
#define AGENTS_H

#include "portmanteau.h"
#include "utils.h"
#include "game.h"

struct game *random_play_single_game(void);

struct game *greedy_play_single_game(void);
void greedy_free(void);

#endif
