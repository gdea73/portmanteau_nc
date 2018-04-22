CFLAGS=-Wall -g -std=c11 -I./inc
LDFLAGS=-lncurses -lmenu -lm

all: portmanteau

portmanteau: letter.c mainMenu.c game.c scores.c portmanteau.c words.c agents/*.c

clean:
	rm -f portmanteau
