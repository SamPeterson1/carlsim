#include "Search.h"

int positionsSearched = 0;
uint16_t bestMoves[16];

int contains(uint16_t *arr, uint16_t a, int len) {
    for(int i = 0; i < len; i ++) {
        if(arr[i] == a) return TRUE;
    }
    return FALSE;
}

void orderMoves(uint16_t *moves, int numMoves) {

    int moveEvals[numMoves];
    for(int i = 0; i < numMoves; i ++) {
        moveEvals[i] = evalMove(moves[i]);
    }   

    //insertion sort
    for(int i = 1; i < numMoves; i ++) {
        int j = i;
        int tmp;
        while(j > 0 && moveEvals[j] > moveEvals[j-1]) {
            tmp = moveEvals[j];
            moveEvals[j] = moveEvals[j-1];
            moveEvals[j-1] = tmp;
            tmp = moves[j];
            moves[j] = moves[j-1];
            moves[j-1] = tmp;
            j--;
        }
    }
} 

int quiescense(int alpha, int beta) {

    int eval = evaluate();

    if(eval >= beta) return beta;
    if(eval > alpha) alpha = eval;

    uint16_t moves[256];
    int numMoves = generateLegalMoves(&g_board, moves, GEN_ALL);

    orderMoves(moves, numMoves);
    for(int i = 0; i < numMoves; i ++) {
        if(move_getSpecial(moves[i]) == MOVE_EP_CAPTURE || g_board.pieceCodes[move_dest(moves[i])] != EMPTY) {
            uint16_t lastGameState = g_board.gameState;
            uint16_t lastCapture = makeMove(&g_board, moves[i]);
            eval = -quiescense(-beta, -alpha);
            unMakeMove(&g_board, moves[i], lastCapture, lastGameState);
            if(eval >= beta) return beta;
            if(eval >= alpha) {
                alpha = eval;
            }
        }
    }
    
    return alpha;
}

int negamax(int depth, int alpha, int beta, int distFromRoot) {
    if(depth == 1) {
        return quiescense(alpha, beta);
    } else {
        uint16_t moves[256];
        int numMoves = generateLegalMoves(&g_board, moves, GEN_ALL);
        orderMoves(moves, numMoves);
        if(g_board.mateStatus == CHECK_MATE) return -EVAL_INF/distFromRoot;
        else if(g_board.mateStatus == STALE_MATE) return 0;
        for(int i = 0; i < numMoves; i ++) {
            uint16_t lastGameState = g_board.gameState;
            uint16_t lastCapture = makeMove(&g_board, moves[i]);
            int eval = -negamax(depth-1, -beta, -alpha, distFromRoot + 1);
            unMakeMove(&g_board, moves[i], lastCapture, lastGameState);
            if(eval >= beta) return beta;
            if(eval >= alpha) {
                alpha = eval;
            }
        }
        return alpha;
    }
}

uint16_t findBestMove(int depth) {

    uint16_t bookMove = bk_getMove(&g_board.zobrist);
    if(bookMove != 0) {
        printf("Found position in book");
        return bookMove;
    }

    unsigned long starttime = micros();
    uint16_t moves[256];
    int numMoves = generateLegalMoves(&g_board, moves, GEN_ALL);
    orderMoves(moves, numMoves);
    
    uint16_t bestMove = 0;
    int bestEval = -EVAL_INF;

    for(int i = 0; i < numMoves; i ++) {
        unsigned long time = micros();
        uint16_t lastGameState = g_board.gameState;
        uint16_t lastCapture = makeMove(&g_board, moves[i]);
        int eval = -negamax(depth, -EVAL_INF, EVAL_INF, 1);
        unMakeMove(&g_board, moves[i], lastCapture, lastGameState);
       
        if(eval >= bestEval) {   
            bestEval = eval;
            bestMove = moves[i];
        }
        printf("Evaluated move %d/%d in %f ms\n", i+1, numMoves, (float)(micros() - time)/1000.0);
    }

    printf("Positions searched: %d\nTime: %f ms\n", positionsSearched, (float)(micros() - starttime)/1000.0);
    return bestMove;
}