#ifndef SEARCH_H

#define SEARCH_H

#include "Defines.h"
#include "Types.h"

#include <stdint.h>
#include "Board.h"
#include "MoveGenerator.h"
#include "Eval.h"
#include "Moves.h"
#include "Book.h"
#include "TranspositionTable.h"

uint16_t findBestMove(int depth);

#endif