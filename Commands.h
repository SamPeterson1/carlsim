#include "StrUtil.h"
#include "Moves.h"
#include "Board.h"
#include "MoveGenerator.h"
#include "Search.h"
#include <stdio.h>

int execute(char *input);
void cmdMove(char **args, int argc);
void cmdPrintBoard(char **args, int argc);
void cmdLoad(char **args, int argc);
void cmdPerft(char **args);
void cmdUndo();
void cmdPrintMoves();
void cmdTest();