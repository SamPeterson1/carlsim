#include "TranspositionTable.h"

TTEntry table[TABLE_SIZE];

uint16_t tt_getMove(ZobristKey *key) {
    TTEntry *ent = &table[*key % TABLE_SIZE];
    if(*key == ent->key) {
        return ent->bestMove;
    }
    return MOVE_INVALID;
}

int tt_getEval(ZobristKey *key, int alpha, int beta, int depth) {
    TTEntry *ent = &table[*key % TABLE_SIZE];
    if(*key == ent->key && ent->depth >= depth) {
        if(ent->type == TT_EXACT) {
            return ent->eval;
        } else if(ent->type == TT_LOWERBOUND && ent->eval >= beta) {
            return ent->eval;
        } else if(ent->type == TT_UPPERBOUND && ent->eval <= alpha) {
            return ent->eval;
        }
        return ent->eval;
    }
    return TT_LOOKUP_FAILED;
}

void tt_storeEval(ZobristKey *key, int eval, int depth, int evalType, uint16_t move) {
    TTEntry *ent = &table[*key % TABLE_SIZE];
    ent->type = evalType;
    ent->bestMove = move;
    ent->depth = depth;
    ent->eval = eval;
    ent->key = *key;
}