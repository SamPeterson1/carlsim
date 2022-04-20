#ifndef EVAL_H

#include "Defines.h"
#include "Types.h"

#include <stdint.h>
#include "Board.h"
#include "TranspositionTable.h"
#define EVAL_H

int evaluate();
int evalMove(uint16_t move);

#endif