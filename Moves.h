#ifndef MOVES_H

#define MOVES_H
#include <stdlib.h>
#include "Board.h"

#define MOVE_QUIET 0
#define MOVE_DOUBLE_PAWN_PUSH 0x1000

#define MOVE_KINGSIDE_CASTLE 0x2000
#define MOVE_QUEENSIDE_CASTLE 0x3000

#define MOVE_CAPTURE 0x4000
#define MOVE_EP_CAPTURE 0x5000

#define MOVE_PROMOTION 0x8000
#define MOVE_KNIGHT_PROMOTION 0x8000
#define MOVE_BISHOP_PROMOTION 0x9000
#define MOVE_ROOK_PROMOTION 0xA000
#define MOVE_QUEEN_PROMOTION 0xB000

#define move_origin(move) (move & 0x3F)
#define move_dest(move) ((move & 0xFC0) >> 6)
#define move_isCapture(move) ((move >> 14) & 1)
#define move_isPromotion(move) ((move >> 15) & 1)
#define move_getSpecial(move) (move & 0xF000)

#define move_setOrigin(move, origin) move |= origin
#define move_setDest(move, dest) move |= (dest << 6)
#define move_setCapture(move, capture) move |= (capture << 13)
#define move_setPromotion(move, promotion) move |= (promotion << 14)
#define move_setSpecial(move, special) move |= special

#define create_move(origin, dest, special) (origin | (dest << 6) | special)

void printMoves(uint16_t *moves);
unsigned char makeMove(uint16_t move);
void unMakeMove(uint16_t move, unsigned char lastCapture, uint16_t lastGameState); 
uint16_t parseMove(char *str);
int compareMoves(uint16_t a, uint16_t b);
void toStr(uint16_t move, char *str);

#endif