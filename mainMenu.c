#include "mainMenu.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#define CTRLD 4
#define ENTER 10

MENU *menu;
ITEM **items;
int c, optionsCount, i;
ITEM *selectedItem;

char *options[] = {
					"Play",
					"High Scores",
					"AI: Random Agent",
					"AI: Sequential Agent",
					"Exit"
};

void initMenu(void) {
	optionsCount = ARRAY_SIZE(options);
	items = (ITEM **) calloc(optionsCount + 1, sizeof(ITEM *));

	for (i = 0; i < optionsCount; i++) {
		items[i] = new_item(options[i], options[i]);
	}
	items[optionsCount] = (ITEM *) NULL;

	menu = new_menu((ITEM **) items);
	mvprintw(LINES - 2, 0, "Press 'Q' to quit.");
	post_menu(menu);
	refresh();
}

const char *displayMenu() {
	while ((c = getch()) != 'q') {
		switch (c) {
			case KEY_DOWN:
				menu_driver(menu, REQ_DOWN_ITEM);
				break;
			case KEY_UP:
				menu_driver(menu, REQ_UP_ITEM);
				break;
			case 10: /* enter */
				return item_name(current_item(menu));
		}
	}
	return "Exit";
}

void freeMenu(void) {
	for (i = 0; i < optionsCount; i++) {
		free_item(items[i]);
	}
	free_menu(menu);
	endwin();
}
