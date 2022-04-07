#ifndef BOARD_H

#define BOARD_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "Defines.h"
#include "MoveGenerator.h"

#define EMPTY 255

#define WHITE 0
#define BLACK 1

#define PAWN 0
#define KNIGHT 2
#define BISHOP 4
#define ROOK 6
#define QUEEN 8
#define KING 10

#define WHITE_CASTLE_QUEENSIDE_RIGHT 1
#define WHITE_CASTLE_KINGSIDE_RIGHT 2
#define BLACK_CASTLE_QUEENSIDE_RIGHT 4
#define BLACK_CASTLE_KINGSIDE_RIGHT 8

#define STARTING_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

typedef struct Board_s {
    
    unsigned char pieceCodes[64];
    // Bits 0-3 store white and black kingside/queenside castling legality
    // Bits 4-7 file of ep square (starting at 1, so 0 = no ep square)
    // Bit 8 is turn
    // Bits 8-... half move counter
    uint16_t gameState;

    uint64_t pieces[6][2];
    uint64_t bitboard;
    uint64_t colorBitboards[2];

} Board;

extern Board board;

#define board_getTurn() ((board.gameState >> 8) & 1)
#define board_getOpponentTurn() ((~board.gameState >> 8) & 1)
#define board_getPieceType(piece) (piece & 0xE)
#define board_getPieceColor(piece) (piece & 1)
#define board_hasCastleRight(castle) ((board.gameState & castle) == castle)
#define board_getEPFile() (((board.gameState >> 4) & 0x0F) - 1)
#define board_getHalfMoveCounter() (board.gameState >> 8)
#define board_get(i) (board.pieceCodes[i])

#define board_setEPFile(EPFile) board.gameState = ((board.gameState & 0xFF0F) | (((EPFile) + 1) << 4))
#define board_removeCastleRight(castle) board.gameState &= ~(castle)
#define board_giveGastleRight(castle) board.gameState |= castle

#define pawns(turn) board.pieces[0][turn]
#define knights(turn) board.pieces[1][turn]
#define bishops(turn) board.pieces[2][turn]
#define rooks(turn) board.pieces[3][turn]
#define queens(turn) board.pieces[4][turn]
#define kings(turn) board.pieces[5][turn]

char getPieceChar(unsigned char piece);

void loadFENStr(char* str);
void printBoard();
void printInfo();
void printAllPositions();
void setIndex(int index, unsigned char piece);
void clearIndex(int index);

void getSquareStr(int square, char *str);
int getSquareFromStr(char *square);

unsigned char getPieceAtIndex(int index);

#endif