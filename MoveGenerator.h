#ifndef MOVEGEN_H

#include "Moves.h"
#include "Board.h"
#include "Moves.h"
#include "MagicHashTable.h"
#include <sys/time.h>
#include <math.h>

#define MOVEGEN_H
#define TABLE_SIZE (1 << 16)
#define RANK_8_MASK 0xfe00000000000000ULL
#define RANK_1_MASK 0xffULL
#define EDGE_MASK 0x7e7e7e7e7e7e00

#define WHITE_KINGSIDE_CASTLE_MASK 0x70
#define WHITE_QUEENSIDE_CASTLE_MASK 0x1C
#define BLACK_KINGSIDE_CASTLE_MASK 0x7000000000000000
#define BLACK_QUEENSIDE_CASTLE_MASK 0x1c00000000000000

#define NORTH 0
#define NORTHEAST 1
#define EAST 2
#define SOUTHEAST 3
#define SOUTH 4
#define SOUTHWEST 5
#define WEST 6
#define NORTHWEST 7
#define NONE 8

void initMoveGenerator();
int generateLegalMoves(uint16_t *moves);
void printBitboard(uint64_t *bitboard);
void test();
void perft(int depth);


#endif