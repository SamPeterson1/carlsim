#include "Moves.h"

int compareMoves(uint16_t a, uint16_t b) {
    if(move_isPromotion(a)) {
        if(move_getSpecial(a) != move_getSpecial(b)) return FALSE;
    }
    return move_origin(a) == move_origin(b) && move_dest(a) == move_dest(b);
}

void toStr(uint16_t move, char *str) {
    getSquareStr(move_origin(move), str);
    getSquareStr(move_dest(move), str+2);
    if(move_getSpecial(move) == MOVE_KNIGHT_PROMOTION) {
        str[4] = '=';
        str[5] = 'N';
    } else if(move_getSpecial(move) == MOVE_BISHOP_PROMOTION) {
        str[4] = '=';
        str[5] = 'B';
    } else if(move_getSpecial(move) == MOVE_ROOK_PROMOTION) {
        str[4] = '=';
        str[5] = 'R';
    } else if(move_getSpecial(move) == MOVE_QUEEN_PROMOTION) {
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

uint16_t parseAlgebraicMove(Board *board, char *str) {

    int turn = TURN(*board);
    unsigned char piece = turn;
    uint16_t moves[256];
    int numMoves = generateLegalMoves(board, moves, GEN_ALL);
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
            if(board->pieceCodes[dest] == EMPTY) special = MOVE_EP_CAPTURE;
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

        uint64_t candidates = board->pieces[PIECE_TYPE(piece) >> 1][turn];
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

unsigned char makeMove(Board *board, uint16_t move) {
    int special = move_getSpecial(move);
    int origin = move_origin(move);
    int dest = move_dest(move);
    int turn = G_TURN;

    int newEpFile = -1;
    unsigned char lastCapture = board->pieceCodes[dest];

    z_hashGameState(board->gameState, &board->zobrist);

    if(special == MOVE_KINGSIDE_CASTLE) {
        if(turn == WHITE) {
            clearIndex(board, 4); setIndex(board, 6, WHITE | KING);
            clearIndex(board, 7); setIndex(board, 5, WHITE | ROOK);
            REMOVE_CASTLE_RIGHT(*board, WHITE_CASTLE_KINGSIDE_RIGHT | WHITE_CASTLE_QUEENSIDE_RIGHT);
        } else {
            clearIndex(board, 60); setIndex(board, 62, BLACK | KING);
            clearIndex(board, 63); setIndex(board, 61, BLACK | ROOK);
            REMOVE_CASTLE_RIGHT(*board, BLACK_CASTLE_KINGSIDE_RIGHT | BLACK_CASTLE_QUEENSIDE_RIGHT);
        }  
    } else if(special == MOVE_QUEENSIDE_CASTLE) {
        if(turn == WHITE) {
            clearIndex(board, 4); setIndex(board, 2, WHITE | KING);
            clearIndex(board, 0); setIndex(board, 3, WHITE | ROOK);
            REMOVE_CASTLE_RIGHT(*board, WHITE_CASTLE_KINGSIDE_RIGHT | WHITE_CASTLE_QUEENSIDE_RIGHT);
        } else {
            clearIndex(board, 60); setIndex(board, 58, BLACK | KING);
            clearIndex(board, 56); setIndex(board, 59, BLACK | ROOK);
            REMOVE_CASTLE_RIGHT(*board, BLACK_CASTLE_KINGSIDE_RIGHT | BLACK_CASTLE_QUEENSIDE_RIGHT);
        }
    } else if(special == MOVE_EP_CAPTURE) {
        int epFile = EP_FILE(*board);
        int rank = origin/8;
        setIndex(board, dest, board->pieceCodes[origin]);
        setIndex(board, epFile + 8*rank, EMPTY);
        setIndex(board, origin, EMPTY);
        lastCapture = PAWN | (1-turn);
    } else if(move_isPromotion(move)) {
        clearIndex(board, origin);
        if(special == MOVE_KNIGHT_PROMOTION) {
            setIndex(board, dest, KNIGHT | turn);
        } else if(special == MOVE_BISHOP_PROMOTION) {
            setIndex(board, dest, BISHOP | turn);
        } else if(special == MOVE_ROOK_PROMOTION) {
            setIndex(board, dest, ROOK | turn);
        } else if(special == MOVE_QUEEN_PROMOTION) {
            setIndex(board, dest, QUEEN | turn);
        }
    } else {
        if(board->pieceCodes[origin] == (WHITE | KING))
            REMOVE_CASTLE_RIGHT(*board, WHITE_CASTLE_KINGSIDE_RIGHT | WHITE_CASTLE_QUEENSIDE_RIGHT);
        else if(board->pieceCodes[origin] == (BLACK | KING))
            REMOVE_CASTLE_RIGHT(*board, BLACK_CASTLE_KINGSIDE_RIGHT | BLACK_CASTLE_QUEENSIDE_RIGHT);
        
        if(PIECE_TYPE(board->pieceCodes[origin]) == PAWN && abs(origin - dest) == 16) {
            newEpFile = origin & 7;
        } 

        setIndex(board, dest, board->pieceCodes[origin]);
        clearIndex(board, origin);
    }

    int revokedRights = 0;
    if(origin == 0 || dest == 0) revokedRights |= WHITE_CASTLE_QUEENSIDE_RIGHT;
    if(origin == 7 || dest == 7) revokedRights |= WHITE_CASTLE_KINGSIDE_RIGHT;
    if(origin == 56 || dest == 56) revokedRights |= BLACK_CASTLE_QUEENSIDE_RIGHT;
    if(origin == 63 || dest == 63) revokedRights |= BLACK_CASTLE_KINGSIDE_RIGHT;

    REMOVE_CASTLE_RIGHT(*board, revokedRights);
    
    SET_EP_FILE(*board, newEpFile);
    board->gameState ^= 0x100;
    z_hashGameState(board->gameState, &board->zobrist);
    return lastCapture;
}

void unMakeMove(Board *board, uint16_t move, unsigned char lastCapture, uint16_t lastGameState) {
    board->gameState = lastGameState;

    int special = move_getSpecial(move);
    int origin = move_origin(move);
    int dest = move_dest(move);
    int turn = G_TURN;
    z_hashGameState(board->gameState, &board->zobrist);

    if(special == MOVE_KINGSIDE_CASTLE) {
        if(turn == WHITE) {
            clearIndex(board, 6); setIndex(board, 4, WHITE | KING);
            clearIndex(board, 5); setIndex(board, 7, WHITE | ROOK);
        } else {
            clearIndex(board, 62); setIndex(board, 60, BLACK | KING);
            clearIndex(board, 61); setIndex(board, 63, BLACK | ROOK);
        }   
    } else if(special == MOVE_QUEENSIDE_CASTLE) {
        if(turn == WHITE) {
            clearIndex(board, 2); setIndex(board, 4, WHITE | KING);
            clearIndex(board, 3); setIndex(board, 0, WHITE | ROOK);
        } else {
            clearIndex(board, 58); setIndex(board, 60, BLACK | KING);
            clearIndex(board, 59); setIndex(board, 56, BLACK | ROOK);
        }
    } else if(special == MOVE_EP_CAPTURE) {
        int epFile = EP_FILE(*board);
        int rank = origin / 8;
        setIndex(board, origin, board->pieceCodes[dest]);
        setIndex(board, epFile + 8*rank, lastCapture);
        clearIndex(board, dest);
    } else if(move_isPromotion(move)) {
        setIndex(board, dest, lastCapture);
        setIndex(board, origin, turn | PAWN);
    } else {
        setIndex(board, origin, board->pieceCodes[dest]);
        setIndex(board, dest, lastCapture);
    }

    z_hashGameState(board->gameState, &board->zobrist);
}
