#ifndef TYPES_H

#define TYPES_H

#include <stdint.h>

typedef uint64_t ZobristKey;

typedef struct Board_s {
    
    unsigned char pieceCodes[64];
    // Bits 0-3 store white and black kingside/queenside castling legality
    // Bits 4-7 file of ep square (starting at 1, so 0 = no ep square)
    // Bit 8 is turn
    // Bits 9-15 is halfmove counter
    uint16_t gameState;

    uint64_t pieces[6][2];
    uint64_t bitboard;
    uint64_t colorBitboards[2];

    ZobristKey zobrist;

    int mateStatus;

} Board;

typedef struct MHTItem_s {
    uint16_t *key;
    uint64_t *value;
} MHTItem;

typedef struct MagicHashTable_s {
    int bits;
    uint64_t magic;
    MHTItem **items;
} MagicHashTable;

typedef struct TTEntry_s {
    ZobristKey key;
    uint16_t bestMove;

    int eval;
    int depth;
    int type;
} TTEntry;

typedef struct KeyValuePair_s
{
    char key[32];
    char value[32];
} KeyValuePair;

#endif