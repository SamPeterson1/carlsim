#ifndef MOVEGEN_H

#include "Moves.h"
#include "Board.h"
#include "Moves.h"
#include "MagicHashTable.h"
#include <sys/time.h>
#include <math.h>

#define MOVEGEN_H


void initMoveGenerator(void);

int inCheck(int turn);
int generatePseudoLegalMoves(uint16_t *moves, int genType);
int generateLegalMoves(uint16_t *moves, int genType);
void updateMateStatus(void);
void printBitboard(uint64_t *bitboard);
void perft(int depth);

extern uint64_t pawnAttacks[64][2];

#endif