#include "Moves.h"

int compareMoves(uint16_t a, uint16_t b) {
    if(MOVE_IS_PROMOTION(a)) {
        if(MOVE_GET_SPECIAL(a) != MOVE_GET_SPECIAL(b)) return FALSE;
    }
    return  MOVE_ORIGIN(a) ==  MOVE_ORIGIN(b) &&  MOVE_DEST(a) ==  MOVE_DEST(b);
}

void toStr(uint16_t move, char *str) {
    getSquareStr( MOVE_ORIGIN(move), str);
    getSquareStr( MOVE_DEST(move), str+2);
    if(MOVE_GET_SPECIAL(move) == MOVE_KNIGHT_PROMOTION) {
        str[4] = '=';
        str[5] = 'N';
    } else if(MOVE_GET_SPECIAL(move) == MOVE_BISHOP_PROMOTION) {
        str[4] = '=';
        str[5] = 'B';
    } else if(MOVE_GET_SPECIAL(move) == MOVE_ROOK_PROMOTION) {
        str[4] = '=';
        str[5] = 'R';
    } else if(MOVE_GET_SPECIAL(move) == MOVE_QUEEN_PROMOTION) {
        str[4] = '=';
        str[5] = 'Q';
    } else {
        str[4] = '\0';
        str[5] = '\0';
    }
    str[6] = '\0';
}

void printMoves(uint16_t *moves) {
    int count = 0;
    do {
        char str[7];
        toStr(*moves, str);
        count ++;
        printf("%s\n", str);
    } while(*++moves != 0);
    printf("Count: %d\n", count);
}

static inline int popCount(uint64_t a) {
    return __builtin_popcountll(a);
}

static inline int getLSB(uint64_t *a) {
    return __builtin_ctzll(*a);
}

static inline int popLSB(uint64_t *a) {
    int index = getLSB(a);
    *a &= *a - 1;
    return index;
}

uint16_t parseAlgebraicMove(char *str) {

    int turn = TURN;
    unsigned char piece = turn;
    uint16_t moves[256];
    int numMoves = generateLegalMoves(moves, GEN_ALL);
    int origin = 0;
    int dest = 0;
    int special = 0;

    if(strcmp(str, "O-O-O") == 0 || strcmp(str, "O-O-O+") == 0) {
        if(turn == WHITE) return create_move(4, 2, MOVE_QUEENSIDE_CASTLE);
        else return create_move(60, 58, MOVE_QUEENSIDE_CASTLE);
    } else if(strcmp(str, "O-O") == 0 || strcmp(str, "O-O+") == 0) {
        if(turn == WHITE) return create_move(4, 6, MOVE_KINGSIDE_CASTLE);
        else return create_move(60, 62, MOVE_KINGSIDE_CASTLE);
    }

    switch(str[0]) {
        case 'N':
            piece |= KNIGHT;
            break;
        case 'B':
            piece |= BISHOP;
            break;
        case 'R':
            piece |= ROOK;
            break;
        case 'Q':
            piece |= QUEEN;
            break;
        case 'K':
            piece |= KING;
            break;
    }

    int isCapture = FALSE;
    int isCheck = FALSE;
    int len = 0;
    while(str[++len] != '\0') {
        if(str[len] == '+') isCheck = TRUE;
        if(str[len] == 'x') isCapture = TRUE;
    }

    if(isCheck) len--;
    int lenWithoutX = len;
    if(isCapture) lenWithoutX --;

    //exe4 or f5
    if(PIECE_TYPE(piece) == PAWN) {
        if(isCapture) {
            int destFile = getFile(str[2]);
            int destRank = getRank(str[3]);
            dest = destRank * 8 + destFile;
            if(g_board.pieceCodes[dest] == EMPTY) special = MOVE_EP_CAPTURE;
            if(turn == WHITE) origin = getFile(str[0]) + 8*(destRank-1);
            else origin = getFile(str[0]) + 8*(destRank+1);
        } else if(len == 2) {
            int destFile = getFile(str[0]);
            int destRank = getRank(str[1]);
            dest = destRank * 8 + destFile;
            if(turn == WHITE) {
                uint16_t move = create_move(dest - 8, dest, MOVE_QUIET);
                for(int i = 0; i < numMoves; i ++) if(compareMoves(move, moves[i])) return move;
                move = create_move(dest - 16, dest, MOVE_QUIET);
                for(int i = 0; i < numMoves; i ++) if(compareMoves(move, moves[i])) return move;
            } else {
                uint16_t move = create_move(dest + 8, dest, MOVE_QUIET);
                for(int i = 0; i < numMoves; i ++) if(compareMoves(move, moves[i])) return move;
                move = create_move(dest + 16, dest, MOVE_QUIET);
                for(int i = 0; i < numMoves; i ++) if(compareMoves(move, moves[i])) return move;
            }
        } else {
            int destFile = getFile(str[0]);
            int destRank = getRank(str[1]);
            dest = destRank * 8 + destFile;
            if(turn == WHITE) origin = dest - 8;
            else origin = dest + 8;
            if(str[3] == 'N') special = MOVE_KNIGHT_PROMOTION;
            else if(str[3] == 'B') special = MOVE_BISHOP_PROMOTION;
            else if(str[3] == 'R') special = MOVE_ROOK_PROMOTION;
            else if(str[3] == 'Q') special = MOVE_QUEEN_PROMOTION;
        }
    } else {
        //Naxc4
        char destChar[2] = {str[len-2], str[len-1]};
        dest = getSquareFromStr(destChar);

        uint64_t candidates = g_board.pieces[PIECE_TYPE(piece) >> 1][turn];
        int specRank = -1;
        int specFile = -1;

        if(lenWithoutX == 4) {
            char c = str[1];
            if(c > '0' && c < '9') specRank = getRank(c);
            else if(c >= 'a' && c <= 'h') specFile = getFile(c);
        } else if(lenWithoutX == 5) {
            specRank = getRank(str[1]);
            specFile = getFile(str[2]);
        }
        
        while(candidates) {
            int square = popLSB(&candidates);
            int rank = square / 8;
            int file = square % 8;
            if((file == specFile || specFile == -1) && (rank == specRank || specRank == -1)) {
                uint16_t move = create_move(square, dest, MOVE_QUIET);
                for(int i = 0; i < numMoves; i ++) {
                    if(compareMoves(move, moves[i])) {
                        origin = square;
                        break;
                    }
                }
            }
        }

    }

    return create_move(origin, dest, special);
}

uint16_t parseMove(char *str) {
    int origin = getSquareFromStr(str);
    int dest = getSquareFromStr(str + 2);
    int special = 0;
    if(strlen(str) > 5) {
        switch(str[5]) {
            case 'N':
                special = MOVE_KNIGHT_PROMOTION;
                break;
            case 'B':
                special = MOVE_BISHOP_PROMOTION;
                break;
            case 'R':
                special = MOVE_ROOK_PROMOTION;
                break;
            case 'Q':
                special = MOVE_QUEEN_PROMOTION;
                break;
        }
    }

    if(g_board.pieceCodes[origin] == (WHITE | KING) && dest == 6) {
        special = MOVE_KINGSIDE_CASTLE;
    } else if(g_board.pieceCodes[origin] == (WHITE | KING) && dest == 2) {
        printf("queen castle\n");
        special = MOVE_QUEENSIDE_CASTLE;
    } else if(g_board.pieceCodes[origin] == (BLACK | KING) && dest == 62) {
        special = MOVE_KINGSIDE_CASTLE;
    } else if(g_board.pieceCodes[origin] == (BLACK | KING) && dest == 58) {
        special = MOVE_QUEENSIDE_CASTLE;
    } else if(PIECE_TYPE(g_board.pieceCodes[origin]) == PAWN && g_board.pieceCodes[dest] == EMPTY && (abs(origin - dest) == 7 || abs(origin - dest) == 9)) {
        special = MOVE_EP_CAPTURE;
    }

    return create_move(origin, dest, special);
}

unsigned char makeMove(uint16_t move) {
    int special = MOVE_GET_SPECIAL(move);
    int origin =  MOVE_ORIGIN(move);
    int dest =  MOVE_DEST(move);
    int turn = TURN;

    int newEpFile = -1;
    unsigned char lastCapture = g_board.pieceCodes[dest];

    z_hashGameState(g_board.gameState, &g_board.zobrist);

    if(special == MOVE_KINGSIDE_CASTLE) {
        if(turn == WHITE) {
            clearIndex(4); setIndex(6, WHITE | KING);
            clearIndex(7); setIndex(5, WHITE | ROOK);
            REMOVE_CASTLE_RIGHT(WHITE_CASTLE_KINGSIDE_RIGHT | WHITE_CASTLE_QUEENSIDE_RIGHT);
        } else {
            clearIndex(60); setIndex(62, BLACK | KING);
            clearIndex(63); setIndex(61, BLACK | ROOK);
            REMOVE_CASTLE_RIGHT(BLACK_CASTLE_KINGSIDE_RIGHT | BLACK_CASTLE_QUEENSIDE_RIGHT);
        }  
    } else if(special == MOVE_QUEENSIDE_CASTLE) {
        if(turn == WHITE) {
            clearIndex(4); setIndex(2, WHITE | KING);
            clearIndex(0); setIndex(3, WHITE | ROOK);
            REMOVE_CASTLE_RIGHT(WHITE_CASTLE_KINGSIDE_RIGHT | WHITE_CASTLE_QUEENSIDE_RIGHT);
        } else {
            clearIndex(60); setIndex(58, BLACK | KING);
            clearIndex(56); setIndex(59, BLACK | ROOK);
            REMOVE_CASTLE_RIGHT(BLACK_CASTLE_KINGSIDE_RIGHT | BLACK_CASTLE_QUEENSIDE_RIGHT);
        }
    } else if(special == MOVE_EP_CAPTURE) {
        int epFile = EP_FILE;
        int rank = origin/8;
        setIndex(dest, g_board.pieceCodes[origin]);
        setIndex(epFile + 8*rank, EMPTY);
        setIndex(origin, EMPTY);
        lastCapture = PAWN | (1-turn);
    } else if(MOVE_IS_PROMOTION(move)) {
        clearIndex(origin);
        if(special == MOVE_KNIGHT_PROMOTION) {
            setIndex(dest, KNIGHT | turn);
        } else if(special == MOVE_BISHOP_PROMOTION) {
            setIndex(dest, BISHOP | turn);
        } else if(special == MOVE_ROOK_PROMOTION) {
            setIndex(dest, ROOK | turn);
        } else if(special == MOVE_QUEEN_PROMOTION) {
            setIndex(dest, QUEEN | turn);
        }
    } else {
        if(g_board.pieceCodes[origin] == (WHITE | KING))
            REMOVE_CASTLE_RIGHT(WHITE_CASTLE_KINGSIDE_RIGHT | WHITE_CASTLE_QUEENSIDE_RIGHT);
        else if(g_board.pieceCodes[origin] == (BLACK | KING))
            REMOVE_CASTLE_RIGHT(BLACK_CASTLE_KINGSIDE_RIGHT | BLACK_CASTLE_QUEENSIDE_RIGHT);
        
        if(PIECE_TYPE(g_board.pieceCodes[origin]) == PAWN && abs(origin - dest) == 16) {
            newEpFile = origin & 7;
        } 

        setIndex(dest, g_board.pieceCodes[origin]);
        clearIndex(origin);
    }

    int revokedRights = 0;
    if(origin == 0 || dest == 0) revokedRights |= WHITE_CASTLE_QUEENSIDE_RIGHT;
    if(origin == 7 || dest == 7) revokedRights |= WHITE_CASTLE_KINGSIDE_RIGHT;
    if(origin == 56 || dest == 56) revokedRights |= BLACK_CASTLE_QUEENSIDE_RIGHT;
    if(origin == 63 || dest == 63) revokedRights |= BLACK_CASTLE_KINGSIDE_RIGHT;

    REMOVE_CASTLE_RIGHT(revokedRights);
    
    SET_EP_FILE(newEpFile);
    g_board.gameState ^= 256;
    z_hashGameState(g_board.gameState, &g_board.zobrist);

    if(lastCapture == EMPTY) {
        SET_HALFMOVE_COUNTER(HALFMOVE_COUNTER + 1);
    } else {
        SET_HALFMOVE_COUNTER(0);
    }

    return lastCapture;
}

void unMakeMove(uint16_t move, unsigned char lastCapture, uint16_t lastGameState) {
    g_board.gameState = lastGameState;

    int special = MOVE_GET_SPECIAL(move);
    int origin =  MOVE_ORIGIN(move);
    int dest =  MOVE_DEST(move);
    int turn = TURN;
    z_hashGameState(g_board.gameState, &g_board.zobrist);

    if(special == MOVE_KINGSIDE_CASTLE) {
        if(turn == WHITE) {
            clearIndex(6); setIndex(4, WHITE | KING);
            clearIndex(5); setIndex(7, WHITE | ROOK);
        } else {
            clearIndex(62); setIndex(60, BLACK | KING);
            clearIndex(61); setIndex(63, BLACK | ROOK);
        }   
    } else if(special == MOVE_QUEENSIDE_CASTLE) {
        if(turn == WHITE) {
            clearIndex(2); setIndex(4, WHITE | KING);
            clearIndex(3); setIndex(0, WHITE | ROOK);
        } else {
            clearIndex(58); setIndex(60, BLACK | KING);
            clearIndex(59); setIndex(56, BLACK | ROOK);
        }
    } else if(special == MOVE_EP_CAPTURE) {
        int epFile = EP_FILE;
        int rank = origin / 8;
        setIndex(origin, g_board.pieceCodes[dest]);
        setIndex(epFile + 8*rank, lastCapture);
        clearIndex(dest);
    } else if(MOVE_IS_PROMOTION(move)) {
        setIndex(dest, lastCapture);
        setIndex(origin, turn | PAWN);
    } else {
        setIndex(origin, g_board.pieceCodes[dest]);
        setIndex(dest, lastCapture);
    }

    z_hashGameState(g_board.gameState, &g_board.zobrist);
}
