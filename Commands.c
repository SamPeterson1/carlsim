#include "Commands.h"

uint16_t lastMove;
uint16_t lastGameState;
unsigned char lastCapture;

int cmd_execute(char *input) {
    char **args = initBuffer();
    int argc;
    parse(input, args, &argc);

    if(strcmp(args[0], "quit") == 0) {
        return TRUE;
    }

    if(strcmp(args[0], "move") == 0) {
        cmd_move(args, argc);
    } else if(strcmp(args[0], "board") == 0) {
        cmd_printBoard(args, argc);
    } else if(strcmp(args[0], "load") == 0) {
        cmd_load(args, argc);
    } else if(strcmp(args[0], "moves") == 0) {
        cmd_printMoves();
    } else if(strcmp(args[0], "undo") == 0) {
        cmd_undo();
    } else if(strcmp(args[0], "perft") == 0) {
        cmd_perft(args);
    } else if(strcmp(args[0], "ai") == 0) {
        uint16_t move;
        if(argc == 2) move = findBestMove(atoi(args[1]));
        else move = findBestMove(6);
        lastGameState = g_board.gameState;
        lastCapture = makeMove(&g_board, move);
        printBoard(&g_board);
    }  else if(strcmp(args[0], "eval") == 0) {
        printf("Eval: %d\n", evaluate());
    } else if(strcmp(args[0], "mate") == 0) {
        updateMateStatus();
        printf("Mate status %d\n", g_board.mateStatus);
    } else if(strcmp(args[0], "pseudo") == 0) {
        uint16_t *moves = (uint16_t *) calloc(256, sizeof(uint16_t));
        int num = generatePseudoLegalMoves(moves, GEN_ALL);
        printf("%d\n", num);
    } else if(strcmp(args[0], "key") == 0) {
        ZobristKey key = 0;
        z_getKey(&g_board, &key);
        if(key != g_board.zobrist) {
            printf("Error. Keys don't match.\n");
        }
        printf("Key: %ld\n", g_board.zobrist);
    } else if(strcmp(args[0], "san") == 0) {
        for(int i = 1; i < argc; i ++) {
            uint16_t move = parseAlgebraicMove(&g_board, args[i]);
            makeMove(&g_board, move);
        }
        printBoard(&g_board);
    } else if(strcmp(args[0], "book") == 0) {
        uint16_t move = bk_getMove(&g_board.zobrist);
        if(move == 0) {
            printf("Couldn't find position in book\n");
        } else {
            makeMove(&g_board, move);
            printBoard(&g_board);
        }
    }

    return FALSE;
}

void cmd_move(char **args, int argc) {
    for(int i = 1; i < argc; i ++) {
        uint16_t *moves = (uint16_t *) calloc(256, sizeof(uint16_t));
        generateLegalMoves(&g_board, moves, GEN_ALL);
        uint16_t move = parseMove(args[i]);
        int legal = FALSE;
        do {
            if(compareMoves(move, *moves)) {
                legal = TRUE;
                break;
            }
        } while (*++moves != 0);
        
        if(legal) {
            printf("Move eval: %d\n", evalMove(move));
            makeMove(&g_board, parseMove(args[i]));
            printBoard(&g_board);
        } else {
            printf("Illegal move\n");
        }
    }
}

void cmd_perft(char **args) {
    int depth = atoi(args[1]);
    perft(depth);
}

void cmd_undo() {
    unMakeMove(&g_board, lastMove, lastCapture, lastGameState);
}

void cmd_printBoard(char **args, int argc) {
    printBoard(&g_board);
    for(int i = 1; i < argc; i ++) {
        if(strcmp(args[i], "info") == 0) {
            printInfo(&g_board);
        }
    }
}

void cmd_load(char **args, int argc) {
    if(argc == 2) loadFENStr(&g_board, args[1]);
}

void cmd_printMoves() {
    uint16_t *moves = (uint16_t *) calloc(256, sizeof(uint16_t));

    generateLegalMoves(&g_board, moves, GEN_ALL);
    printMoves(moves);
}