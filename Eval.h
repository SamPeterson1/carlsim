#ifndef EVAL_H

#include "Defines.h"
#include "Types.h"

#include <stdint.h>
#include "Board.h"
#include "Log.h"
#include "TranspositionTable.h"
#define EVAL_H

int evaluate(void);
int evalMove(uint16_t move);

#endif