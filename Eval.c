#include "Eval.h"

const int pawns[2][64] = {
    {
        0,  0,  0,  0,  0,  0,  0,  0,
        50, 50, 50, 50, 50, 50, 50, 50,
        10, 10, 20, 30, 30, 20, 10, 10,
        5,  5, 10, 25, 25, 10,  5,  5,
        0,  0,  0, 20, 20,  0,  0,  0,
        5, -5,-10,  0,  0,-10, -5,  5,
        5, 10, 10,-20,-20, 10, 10,  5,
        0,  0,  0,  0,  0,  0,  0,  0
    },
    {
        0,  0,  0,  0,  0,  0,  0,  0,
        5, 10, 10,-20,-20, 10, 10,  5,
        5, -5,-10,  0,  0,-10, -5,  5,
        0,  0,  0, 20, 20,  0,  0,  0,
        5,  5, 10, 25, 25, 10,  5,  5,
        10, 10, 20, 30, 30, 20, 10, 10,
        50, 50, 50, 50, 50, 50, 50, 50,
        0,  0,  0,  0,  0,  0,  0,  0,
    },
};

const int knights[2][64] = {
    {
        -50,-40,-30,-30,-30,-30,-40,-50,
        -40,-20,  0,  0,  0,  0,-20,-40,
        -30,  0, 10, 15, 15, 10,  0,-30,
        -30,  5, 15, 20, 20, 15,  5,-30,
        -30,  0, 15, 20, 20, 15,  0,-30,
        -30,  5, 10, 15, 15, 10,  5,-30,
        -40,-20,  0,  5,  5,  0,-20,-40,
        -50,-40,-30,-30,-30,-30,-40,-50,
    },
    {
        -50,-40,-30,-30,-30,-30,-40,-50,
        -40,-20,  0,  5,  5,  0,-20,-40,
        -30,  5, 10, 15, 15, 10,  5,-30,
        -30,  0, 15, 20, 20, 15,  0,-30,
        -30,  5, 15, 20, 20, 15,  5,-30,
        -30,  0, 10, 15, 15, 10,  0,-30,
        -40,-20,  0,  0,  0,  0,-20,-40,
        -50,-40,-30,-30,-30,-30,-40,-50,
    }
};

const int bishops[2][64] = {
    {
        -20,-10,-10,-10,-10,-10,-10,-20,
        -10,  0,  0,  0,  0,  0,  0,-10,
        -10,  0,  5, 10, 10,  5,  0,-10,
        -10,  5,  5, 10, 10,  5,  5,-10,
        -10,  0, 10, 10, 10, 10,  0,-10,
        -10, 10, 10, 10, 10, 10, 10,-10,
        -10,  5,  0,  0,  0,  0,  5,-10,
        -20,-10,-10,-10,-10,-10,-10,-20,
    }, 
    {
        -20,-10,-10,-10,-10,-10,-10,-20,
        -10,  5,  0,  0,  0,  0,  5,-10,
        -10, 10, 10, 10, 10, 10, 10,-10,
        -10,  0, 10, 10, 10, 10,  0,-10,
        -10,  5,  5, 10, 10,  5,  5,-10,
        -10,  0,  5, 10, 10,  5,  0,-10,
        -10,  0,  0,  0,  0,  0,  0,-10,
        -20,-10,-10,-10,-10,-10,-10,-20,
    }
    
};

const int rooks[2][64] = {
    {
        0,  0,  0,  0,  0,  0,  0,  0,
        5, 10, 10, 10, 10, 10, 10,  5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        0,  -10,  0,  5,  5,  0,  -10,  0
    },
    {
        0,  -10,  0,  5,  5,  0,  -10,  0
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        5, 10, 10, 10, 10, 10, 10,  5,
        0,  0,  0,  0,  0,  0,  0,  0,
    }
    
};

const int queens[2][64] = {
    {
        -20,-10,-10, -5, -5,-10,-10,-20,
        -10,  0,  0,  0,  0,  0,  0,-10,
        -10,  0,  5,  5,  5,  5,  0,-10,
        -5,  0,  5,  5,  5,  5,  0, -5,
        0,  0,  5,  5,  5,  5,  0, -5,
        -10,  5,  5,  5,  5,  5,  0,-10,
        -10,  0,  5,  0,  0,  0,  0,-10,
        -20,-10,-10, -5, -5,-10,-10,-20
    },
    {
        -20,-10,-10, -5, -5,-10,-10,-20,
        -10,  0,  5,  0,  0,  0,  0,-10,
        -10,  5,  5,  5,  5,  5,  0,-10,
        0,  0,  5,  5,  5,  5,  0, -5,
        -5,  0,  5,  5,  5,  5,  0, -5,
        -10,  0,  5,  5,  5,  5,  0,-10,
        -10,  0,  0,  0,  0,  0,  0,-10,
        -20,-10,-10, -5, -5,-10,-10,-20,
    }
    
};

const int kings[2][64] = {
    {
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -20,-30,-30,-40,-40,-30,-30,-20,
        -10,-20,-20,-20,-20,-20,-20,-10,
        20, 20,  0,  0,  0,  0, 20, 20,
        20, 30, 10,  0,  0, 10, 30, 20
    },
    {
        20, 30, 10,  0,  0, 10, 30, 20,
        20, 20,  0,  0,  0,  0, 20, 20,
        -10,-20,-20,-20,-20,-20,-20,-10,
        -20,-30,-30,-40,-40,-30,-30,-20,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
    }
};

const int pieceValues[5] = {PAWN_VALUE, KNIGHT_VALUE, BISHOP_VALUE, ROOK_VALUE, QUEEN_VALUE};

static inline int popCount(uint64_t *bitboard) {
    return __builtin_popcountll(*bitboard);
}

static inline int popLSB(uint64_t *a) {
    int index = __builtin_ctzll(*a);
    *a &= *a - 1;
    return index;
}

static inline int getLSB(uint64_t *a) {
    return __builtin_ctzll(*a);
}

int countMaterial() {
    int turn = G_TURN;
    return popCount(&G_PAWNS(turn)) + 3*popCount(&G_KNIGHTS(turn)) + 3*popCount(&G_BISHOPS(turn)) + 5*popCount(&G_ROOKS(turn)) + 9*popCount(&G_QUEENS(turn));
}

int evalMove(uint16_t move) {
    int eval = 0;
    int turn = G_TURN;
    int dest = move_dest(move);
    int origin = move_origin(move);

    //penalize moving to square attacked by opponent pawn
    if(PIECE_TYPE(g_board.pieceCodes[origin]) != PAWN && (pawnAttacks[dest][turn] & G_PAWNS(1-turn)) != 0) {
        eval -= VALUE(g_board.pieceCodes[origin]);
    }

    //reward high-value captures with low-value pieces
    if(g_board.pieceCodes[dest] != EMPTY) {
        eval += 10*VALUE(g_board.pieceCodes[dest]) - VALUE(g_board.pieceCodes[origin]);
    }

    //reward promotions
    if(move_isPromotion(move)) {
        int special = move_getSpecial(move);
        if(special == MOVE_KNIGHT_PROMOTION) {
            eval += KNIGHT_VALUE;
        } else if(special == MOVE_BISHOP_PROMOTION) {
            eval += BISHOP_VALUE;
        } else if(special == MOVE_ROOK_PROMOTION) {
            eval += ROOK_VALUE;
        } else if(special == MOVE_QUEEN_PROMOTION) {
            eval += QUEEN_VALUE;
        }
    }

    return eval;
    //TODO: reward checks?
} 

int evaluate() {

    updateMateStatus();
    if(g_board.mateStatus == STALE_MATE) return 0;
    else if(g_board.mateStatus == CHECK_MATE) return (G_TURN == WHITE) ? -EVAL_INF : EVAL_INF;   

    int eval = 0;
    uint64_t pieces = 0;
    for(int turn = 0; turn <= 1; turn ++) {
        int perspective = (turn == WHITE) ? 1 : -1;
        pieces = G_PAWNS(turn);
        while(pieces) eval += (PAWN_VALUE + (int)(1.5*(double)pawns[turn][popLSB(&pieces)])) * perspective;
        pieces = G_KNIGHTS(turn);
        while(pieces) eval += (KNIGHT_VALUE + (int)(1.5*(double)knights[turn][popLSB(&pieces)])) * perspective;
        pieces = G_BISHOPS(turn);
        while(pieces) eval += (BISHOP_VALUE + (int)(1.5*(double)bishops[turn][popLSB(&pieces)])) * perspective;
        pieces = G_ROOKS(turn);
        while(pieces) eval += (ROOK_VALUE + (int)(1.5*(double)rooks[turn][popLSB(&pieces)])) * perspective;
        pieces = G_QUEENS(turn);
        while(pieces) eval += (QUEEN_VALUE + (int)(1.5*(double)queens[turn][popLSB(&pieces)])) * perspective;
        eval += kings[turn][getLSB(&G_KINGS(turn))];
    }

    return (G_TURN == WHITE) ? eval : -eval;
}