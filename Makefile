CFLAGS=-Wall -g -std=c11 -I./inc -I./ai
LDFLAGS=-lncurses -lmenu -lm

PORTMANTEAU_FILES=utils.c letter.c mainMenu.c game.c scores.c words.c
AI_FILES=ai/*.c

all: portmanteau ai

portmanteau: $(PORTMANTEAU_FILES)
	gcc $(CFLAGS) -o portmanteau $(PORTMANTEAU_FILES) portmanteau.c $(LDFLAGS)

ai: $(AI_FILES) $(PORTMANTEAU_FILES)
	gcc $(CFLAGS) -o portmanteau_ai $(PORTMANTEAU_FILES) $(AI_FILES) $(LDFLAGS) 

clean:
	rm -f portmanteau portmanteau_ai
