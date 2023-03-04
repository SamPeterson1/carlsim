#ifndef BOOK_H

#define BOOK_H

#include "Types.h"
#include "Defines.h"
#include "Moves.h"
#include "Log.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>

void bk_parseAll(char *dir);
void bk_parsePGN(char *path);
uint16_t bk_getMove(ZobristKey *key);

#endif