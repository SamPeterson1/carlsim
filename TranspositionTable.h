#ifndef TT_H

#define TT_H

#include "Defines.h"
#include "Types.h"

uint16_t tt_getMove(ZobristKey *key);
int tt_getEval(ZobristKey *key, int alpha, int beta, int depth);
void tt_storeEval(ZobristKey *key, int eval, int depth, int evalType, uint16_t move);
void tt_storeMove(ZobristKey *key, uint16_t move, int depth);

#endif