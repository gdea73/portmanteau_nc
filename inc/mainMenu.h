#ifndef MAINMENU_H
#define MAINMENU_H

#include <ncurses.h>
#include <stdlib.h>
#include <menu.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#define CTRLD 4
#define ENTER 10

extern char *options[];

MENU *menu;
ITEM **items;
int c, optionsCount, i;
ITEM *selectedItem;

void initMenu(void);

const char * displayMenu(void);

void freeMenu(void);

#endif
