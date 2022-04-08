#ifndef SEARCH_H

#define SEARCH_H
#include <stdint.h>
#include "Board.h"
#include "MoveGenerator.h"
#include "Eval.h"
#include "Moves.h"

uint16_t findBestMove();
void s_init();

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#endif