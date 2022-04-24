/*

    Carlsim Chess Engine
    Copyright (C) 2022 Sam Peterson

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.

*/


#include "Search.h"

int positionsSearched = 0;
uint16_t bestMove;

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

    positionsSearched ++;
    int eval = evaluate();

    if(eval >= beta) return beta;
    if(eval > alpha) alpha = eval;

    uint16_t moves[256];
    int numMoves = generateLegalMoves(moves, GEN_ALL);

    orderMoves(moves, numMoves);
    for(int i = 0; i < numMoves; i ++) {
        if(MOVE_GET_SPECIAL(moves[i]) == MOVE_EP_CAPTURE || g_board.pieceCodes[ MOVE_DEST(moves[i])] != EMPTY) {
            uint16_t lastGameState = g_board.gameState;
            uint16_t lastCapture = makeMove(moves[i]);
            eval = -quiescense(-beta, -alpha);
            unMakeMove(moves[i], lastCapture, lastGameState);
            if(eval >= beta) return beta;
            if(eval >= alpha) {
                alpha = eval;
            }
        }
    }
    
    return alpha;
}

int negamax(int depth, int alpha, int beta, int distFromRoot) {
    if(depth == 0) {
        return quiescense(alpha, beta);
    } else {

        int ttEval = tt_getEval(&g_board.zobrist, alpha, beta, depth);
        if(ttEval != TT_LOOKUP_FAILED) {
            return ttEval;
        }

        uint16_t moves[256];
        uint16_t bestMoveInPos = MOVE_INVALID;
        int evalType = TT_UPPERBOUND;
        int numMoves = generateLegalMoves(moves, GEN_ALL);
        orderMoves(moves, numMoves);
        if(g_board.mateStatus == CHECK_MATE) return -EVAL_INF/distFromRoot;
        else if(g_board.mateStatus == STALE_MATE) return 0;
        for(int i = 0; i < numMoves; i ++) {
            uint16_t lastGameState = g_board.gameState;
            uint16_t lastCapture = makeMove(moves[i]);
            int eval = -negamax(depth-1, -beta, -alpha, distFromRoot + 1);
            positionsSearched ++;
            unMakeMove(moves[i], lastCapture, lastGameState);
            if(distFromRoot == 0) {
                printf("Searched move %d/%d\n", i+1, numMoves);
            }
            if(eval >= beta) {
                tt_storeEval(&g_board.zobrist, eval, depth, TT_LOWERBOUND, moves[i]);
                return beta;
            }
            if(eval > alpha) {
                evalType = TT_EXACT;
                bestMoveInPos = moves[i];
                alpha = eval;
                if(distFromRoot == 0) bestMove = moves[i];
            }
        }

        tt_storeEval(&g_board.zobrist, alpha, depth, evalType, bestMoveInPos);
        return alpha;
    }
}

uint16_t findBestMove(int depth) {

    uint16_t bookMove = bk_getMove(&g_board.zobrist);
    positionsSearched = 0;
    if(bookMove != 0) {
        printf("Found position in book");
        return bookMove;
    }

    unsigned long starttime = micros();
    uint16_t moves[256];
    int numMoves = generateLegalMoves(moves, GEN_ALL);
    orderMoves(moves, numMoves);
    for(int d = 1; d <= depth; d ++) {
        negamax(d, -EVAL_INF-1, EVAL_INF+1, 0);
        printf("Depth searched: %d\n", d);
    }

    float ms = (float)(micros()-starttime)/1000.0;
    printf("\nTime: %f ms\n", ms);
    printf("\nPositions searched: %d\n", positionsSearched);
    printf("%f Nodes/sec\n", positionsSearched / (ms/1000.0));
    return bestMove;
}