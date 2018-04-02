# Portmanteau (```ncurses``` version)

## Description

Portmanteau is a puzzle game in which letters (on "tiles") are dropped into the
columns of the board. When adjacent tiles form a valid English word (according
to the Scrabble dictionary), that word "breaks" and the player is awarded points
based on the rarity of the letters, length of the word, and other factors. In
the event that the entire board is filled (and there are no words to break), the
game ends. Words are read in any orientation except diagonal; any adjacent tiles
are read together as one word.

A lengthier explanation of the rules (score multipliers, letter distribution,
"blank" tiles, &c.) should be added eventually.

## Installation

In the future, I intend to write a PKGBUILD and release this on the Arch User
Repository. In the meantime, there is no installation beyond running ```make```
and using the executable ```portmanteau``` in the build directory.

The user interface was developed with ```ncurses```, so that library is needed
to build the project. There are no other external dependencies.
