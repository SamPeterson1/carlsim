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

uint16_t findBestMove(int depth);
void s_init();

#endif