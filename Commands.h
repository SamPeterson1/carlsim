#include "StrUtil.h"
#include "Moves.h"
#include "Board.h"
#include "MoveGenerator.h"
#include "Search.h"
#include "Book.h"
#include <stdio.h>

int cmd_execute(char *input);
void cmd_move(char **args, int argc);
void cmd_printBoard(char **args, int argc);
void cmd_load(char **args, int argc);
void cmd_perft(char **args);
void cmd_undo(void);
void cmd_printMoves(void);
void cmd_test(void);