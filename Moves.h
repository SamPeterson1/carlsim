#ifndef MOVES_H

#define MOVES_H

#include "Defines.h"
#include "Types.h"
#include "Log.h"

#include <stdlib.h>
#include "Board.h"

void printMoves(uint16_t *moves);
unsigned char makeMove(uint16_t move);
void unMakeMove(uint16_t move, unsigned char lastCapture, uint16_t lastGameState); 
uint16_t parseMove(char *str);
uint16_t parseAlgebraicMove(char *str);
int compareMoves(uint16_t a, uint16_t b);
void toStr(uint16_t move, char *str);

#endif