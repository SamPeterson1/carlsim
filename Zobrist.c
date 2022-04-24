#include "Zobrist.h"

ZobristKey g_zPieceSquareKeys[12][64];
ZobristKey g_zBlackToMoveKey;
ZobristKey g_zCastleKeys[16];
ZobristKey g_zEpFileKeys[8];

ZobristKey lrand(void) {
    return ((ZobristKey) rand() << 32ULL) | ((ZobristKey) rand());
}

void z_init(void) {
    for(int piece = 0; piece < 12; piece ++) {
        for(int square = 0; square < 64; square ++) {
            g_zPieceSquareKeys[piece][square] = lrand();
        }
    }

    g_zBlackToMoveKey = lrand();

    for(int castle = 0; castle < 16; castle ++) {
        g_zCastleKeys[castle] = lrand();
    }

    for(int file = 0; file < 8; file ++) {
        g_zEpFileKeys[file] = lrand();
    }
}

void z_getKey(ZobristKey *key) {
    *key = 0;

    for(int square = 0; square < 64; square ++) {
        if(g_board.pieceCodes[square] != EMPTY)
            *key ^= g_zPieceSquareKeys[g_board.pieceCodes[square]][square];
    }

    z_hashGameState(g_board.gameState, key);
}

void z_hashGameState(uint16_t gameState, ZobristKey *key) {
    int epFile = ((gameState >> 4) & 0x0F) - 1;
    if(epFile != -1) *key ^= g_zEpFileKeys[epFile];

    int castleRights = (gameState & 0xF);
    *key ^= g_zCastleKeys[castleRights];

    if(gameState & 0x100) *key ^= g_zBlackToMoveKey;
}