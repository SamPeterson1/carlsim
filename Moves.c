#include "Moves.h"

int compareMoves(uint16_t a, uint16_t b) {
    return (a == b);
    if(move_isPromotion(a)) {
        if(move_getSpecial(a) != move_getSpecial(b)) return FALSE;
    }
    return (a & 0xFFF) == (b & 0xFFF);
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

    if(board.pieceCodes[origin] == (WHITE | KING) && dest == 6) {
        special = MOVE_KINGSIDE_CASTLE;
    } else if(board.pieceCodes[origin] == (WHITE | KING) && dest == 2) {
        special = MOVE_QUEENSIDE_CASTLE;
    } else if(board.pieceCodes[origin] == (BLACK | KING) && dest == 62) {
        special = MOVE_KINGSIDE_CASTLE;
    } else if(board.pieceCodes[origin] == (BLACK | KING) && dest == 58) {
        special = MOVE_QUEENSIDE_CASTLE;
    } else if(board_getPieceType(board.pieceCodes[origin]) == PAWN && board.pieceCodes[dest] == EMPTY && (abs(origin - dest) == 7 || abs(origin - dest) == 9)) {
        special = MOVE_EP_CAPTURE;
        printf("ep move\n");
    }


    uint16_t move = create_move(origin, dest, special);
    printf("%d\n", move_getSpecial(move));
    return move;
}

unsigned char makeMove(uint16_t move) {
    int special = move_getSpecial(move);
    int origin = move_origin(move);
    int dest = move_dest(move);
    int turn = board_getTurn();

    int newEpFile = -1;
    unsigned char lastCapture = board.pieceCodes[dest];

    if(special == MOVE_KINGSIDE_CASTLE) {
        if(turn == WHITE) {
            clearIndex(4); setIndex(6, WHITE | KING);
            clearIndex(7); setIndex(5, WHITE | ROOK);
            board_removeCastleRight(WHITE_CASTLE_KINGSIDE_RIGHT | WHITE_CASTLE_QUEENSIDE_RIGHT);
        } else {
            clearIndex(60); setIndex(62, BLACK | KING);
            clearIndex(63); setIndex(61, BLACK | ROOK);
            board_removeCastleRight(BLACK_CASTLE_KINGSIDE_RIGHT | BLACK_CASTLE_QUEENSIDE_RIGHT);
        }   
    } else if(special == MOVE_QUEENSIDE_CASTLE) {
        if(turn == WHITE) {
            clearIndex(4); setIndex(2, WHITE | KING);
            clearIndex(0); setIndex(3, WHITE | ROOK);
            board_removeCastleRight(WHITE_CASTLE_KINGSIDE_RIGHT | WHITE_CASTLE_QUEENSIDE_RIGHT);
        } else {
            clearIndex(60); setIndex(58, BLACK | KING);
            clearIndex(56); setIndex(59, BLACK | ROOK);
            board_removeCastleRight(BLACK_CASTLE_KINGSIDE_RIGHT | BLACK_CASTLE_QUEENSIDE_RIGHT);
        }
    } else if(special == MOVE_EP_CAPTURE) {
        int epFile = board_getEPFile();
        int rank = origin/8;
        setIndex(dest, board.pieceCodes[origin]);
        setIndex(epFile + 8*rank, EMPTY);
        setIndex(origin, EMPTY);
        lastCapture = PAWN | (1-turn);
    } else if(move_isPromotion(move)) {
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
        if(board.pieceCodes[origin] == (WHITE | KING))
            board_removeCastleRight(WHITE_CASTLE_KINGSIDE_RIGHT | WHITE_CASTLE_QUEENSIDE_RIGHT);
        else if(board.pieceCodes[origin] == (BLACK | KING))
            board_removeCastleRight(BLACK_CASTLE_KINGSIDE_RIGHT | BLACK_CASTLE_QUEENSIDE_RIGHT);
        
        if(board_getPieceType(board.pieceCodes[origin]) == PAWN && abs(origin - dest) == 16) {
            newEpFile = origin & 7;
        } 

        setIndex(dest, board.pieceCodes[origin]);
        clearIndex(origin);
    }

    int revokedRights = 0;
    if(origin == 0 || dest == 0) revokedRights |= WHITE_CASTLE_QUEENSIDE_RIGHT;
    if(origin == 7 || dest == 7) revokedRights |= WHITE_CASTLE_KINGSIDE_RIGHT;
    if(origin == 56 || dest == 56) revokedRights |= BLACK_CASTLE_QUEENSIDE_RIGHT;
    if(origin == 63 || dest == 63) revokedRights |= BLACK_CASTLE_KINGSIDE_RIGHT;

    board_removeCastleRight(revokedRights);
    
    board_setEPFile(newEpFile);
    board.gameState ^= 256;
    return lastCapture;
}

void unMakeMove(uint16_t move, unsigned char lastCapture, uint16_t lastGameState) {
    board.gameState = lastGameState;

    int special = move_getSpecial(move);
    int origin = move_origin(move);
    int dest = move_dest(move);
    int turn = board_getTurn();
    
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
        int epFile = board_getEPFile();
        int rank = origin / 8;
        setIndex(origin, board.pieceCodes[dest]);
        setIndex(epFile + 8*rank, lastCapture);
        clearIndex(dest);
    } else if(move_isPromotion(move)) {
        setIndex(dest, lastCapture);
        setIndex(origin, turn | PAWN);
    } else {
        setIndex(origin, board.pieceCodes[dest]);
        setIndex(dest, lastCapture);
    }
}
