#include "Search.h"

float negamax(int depth) {

    if(depth == 1) {
        return evaluate();
    } else {
        uint16_t *moves = malloc(sizeof(uint16_t) * 255);
        int numMoves = generateLegalMoves(moves);
        float bestEval = -1000;
        for(int i = 0; i < numMoves; i ++) {
            char moveStr[6];
            toStr(moves[i], moveStr);
            uint16_t lastGameState = board.gameState;
            uint16_t lastCapture = makeMove(moves[i]);
            float eval = -negamax(depth-1);
            bestEval = MAX(eval, bestEval);
            //printf("%s: %f (Best: %f)\n", moveStr, eval, bestEval);
            unMakeMove(moves[i], lastCapture, lastGameState);
        }
        
        return bestEval;
    }
}

uint16_t findBestMove() {
    uint16_t *moves = malloc(sizeof(uint16_t) * 255);
    int numMoves = generateLegalMoves(moves);
    uint16_t bestMove = 0;
    int turn = board_getTurn();
    float bestEval = -1000;
    
    for(int i = 0; i < numMoves; i ++) {
        uint16_t lastGameState = board.gameState;
        uint16_t lastCapture = makeMove(moves[i]);
        //printBoard();
        float eval = -negamax(2);
        //printf("Position eval: %f\n", eval);
        
        if(eval >= bestEval) {
            bestEval = eval;
            bestMove = moves[i];
        }
        unMakeMove(moves[i], lastCapture, lastGameState);
    }

    return bestMove;
}