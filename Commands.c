#include "Commands.h"

uint16_t lastMove;

int execute(char *input) {
    char **args = initBuffer();
    int argc;
    parse(input, args, &argc);

    if(strcmp(args[0], "quit") == 0) {
        return TRUE;
    }

    if(strcmp(args[0], "move") == 0) {
        cmdMove(args, argc);
    } else if(strcmp(args[0], "board") == 0) {
        cmdPrintBoard(args, argc);
    } else if(strcmp(args[0], "load") == 0) {
        cmdLoad(args, argc);
    } else if(strcmp(args[0], "moves") == 0) {
        cmdPrintMoves();
    } else if(strcmp(args[0], "test") == 0) {
        cmdTest();
    } else if(strcmp(args[0], "undo") == 0) {
        cmdUndo();
    } else if(strcmp(args[0], "perft") == 0) {
        cmdPerft(args);
    }

    return FALSE;
}

void cmdMove(char **args, int argc) {
    for(int i = 1; i < argc; i ++) {
        uint16_t *moves = (uint16_t *) calloc(256, sizeof(uint16_t));
        generateLegalMoves(moves);
        uint16_t move = parseMove(args[i]);
        int legal = FALSE;
        do {
            if(compareMoves(move, *moves)) {
                legal = TRUE;
                break;
            }
        } while (*++moves != 0);
        
        if(legal) {
            makeMove(parseMove(args[i]));
            printBoard();
        } else {
            printf("Illegal move\n");
        }
    }
}

void cmdPerft(char **args) {
    int depth = atoi(args[1]);
    perft(depth);
}

void cmdUndo() {
    //unMakeMove(lastMove);
}

unsigned long micros2() {
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return 1000000 * tv.tv_sec + tv.tv_usec;
}

void cmdPrintBoard(char **args, int argc) {
    unsigned long ltime = micros2();
    printBoard();
    printf("%d\n", micros2()-ltime);
    for(int i = 1; i < argc; i ++) {
        if(strcmp(args[i], "info") == 0) {
            printInfo();
        }
    }
}

void cmdLoad(char **args, int argc) {
    if(argc == 2) loadFENStr(args[1]);
}

void cmdPrintMoves() {
    uint16_t *moves = (uint16_t *) calloc(256, sizeof(uint16_t));

    generateLegalMoves(moves);
    printMoves(moves);
}

void cmdTest() {
    test();
}