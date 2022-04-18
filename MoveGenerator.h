#ifndef MOVEGEN_H

#include "Moves.h"
#include "Board.h"
#include "Moves.h"
#include "MagicHashTable.h"
#include <sys/time.h>
#include <math.h>

#define MOVEGEN_H


void initMoveGenerator();

int inCheck(int turn);
int generatePseudoLegalMoves(uint16_t *moves, int genType);
int generateLegalMoves(Board *board, uint16_t *moves, int genType);
void updateMateStatus();
void printBitboard(uint64_t *bitboard);
void test();
void perft(int depth);

extern uint64_t pawnAttacks[64][2];

#endif