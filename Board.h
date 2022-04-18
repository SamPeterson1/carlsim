#ifndef BOARD_H

#define BOARD_H

#include "Defines.h"
#include "Types.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "MoveGenerator.h"
#include "Zobrist.h"

extern Board g_board;

char getPieceChar(unsigned char piece);

void loadFENStr(Board *board, char* str);
void printBoard(Board *board);
void printInfo(Board *board);
void setIndex(Board *board, int index, unsigned char piece);
void clearIndex(Board *board, int index);

void getSquareStr(int square, char *str);
int getSquareFromStr(char *square);
int getRank(char rank);
int getFile(char file);

unsigned char getPieceByte(char pieceChar);
char getPieceChar(unsigned char pieceByte);

unsigned long micros();

#endif