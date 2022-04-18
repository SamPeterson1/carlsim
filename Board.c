#include "Board.h"

Board g_board;

void printBoard(Board *board) {    
    printf("\e[1;1H\e[2J");
    for(int rank = 7; rank >= 0; rank --) {
        printf("    +---+---+---+---+---+---+---+---+\n");
        printf("(%d) ", rank+1);
        for(int file = 0; file < 8; file ++) {
            printf("| %c ", getPieceChar(board->pieceCodes[rank * 8 + file]));
        }
        printf("|\n");
    }
    printf("    +---+---+---+---+---+---+---+---+\n");
    printf("     (a) (b) (c) (d) (e) (f) (g) (h)\n\n");
}

unsigned long micros() {
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return 1000000 * tv.tv_sec + tv.tv_usec;
}

void printInfo(Board *board) {
    printf((G_TURN == WHITE) ? "\nWhite to move\n" : "\nBlack to move\n");
    printf("BlackKCastleRight: %d \n", G_HAS_CASTLE_RIGHT(BLACK_CASTLE_KINGSIDE_RIGHT));
    printf("BlackQCastleRight: %d \n", G_HAS_CASTLE_RIGHT(BLACK_CASTLE_QUEENSIDE_RIGHT));
    printf("WhiteKCastleRight: %d \n", G_HAS_CASTLE_RIGHT(WHITE_CASTLE_KINGSIDE_RIGHT));
    printf("WhiteQCastleRight: %d \n", G_HAS_CASTLE_RIGHT(WHITE_CASTLE_QUEENSIDE_RIGHT));
    printf("En Passant File: %d \n", G_EP_FILE);
    printf("Half Move Clock: %d \n", G_HALFMOVE_COUNTER);
}

unsigned char getPieceByte(char pieceChar) {
    unsigned char pieceByte = 0;
    switch(tolower(pieceChar)) {
        case 'p':
            pieceByte = PAWN;
            break;
        case 'n':
            pieceByte = KNIGHT;
            break;
        case 'b':
            pieceByte = BISHOP;
            break;
        case 'r':
            pieceByte = ROOK;
            break;
        case 'q':
            pieceByte = QUEEN;
            break;
        case 'k':
            pieceByte = KING;
            break;
    }

    if(pieceChar >= 97) pieceByte |= BLACK;

    return pieceByte;
}

char getPieceChar(unsigned char pieceByte) {
    if(pieceByte == EMPTY) return ' ';
    char c = ' ';
    switch(PIECE_TYPE(pieceByte)) {
        case PAWN:
            c = 'p';
            break;
        case KNIGHT:
            c = 'n';
            break;
        case BISHOP:
            c = 'b';
            break;
        case ROOK:
            c = 'r';
            break;
        case QUEEN:
            c = 'q';
            break;
        case KING:
            c = 'k';
            break;
    }
    if(PIECE_COLOR(pieceByte) == WHITE) c -= 32;
    return c;
}

void clearIndex(Board *board, int index) {
    unsigned char piece = board->pieceCodes[index];
    board->bitboard &= ~(1ULL << index);
    board->pieces[PIECE_TYPE(piece) >> 1][PIECE_COLOR(piece)] &= ~(1ULL << index);
    board->colorBitboards[PIECE_COLOR(piece)] &= ~(1ULL << index);

    if(board->pieceCodes[index] != EMPTY) {
        board->zobrist ^= g_zPieceSquareKeys[piece][index];
    }
    board->pieceCodes[index] = EMPTY;
}

void setIndex(Board *board, int index, unsigned char piece) {
    clearIndex(board, index);
    if(piece != EMPTY) {
        board->bitboard |= (1ULL << index);
        board->pieces[PIECE_TYPE(piece) >> 1][PIECE_COLOR(piece)] |= (1ULL << index);
        board->colorBitboards[PIECE_COLOR(piece)] |= (1ULL << index);
        board->pieceCodes[index] = piece;
        board->zobrist ^= g_zPieceSquareKeys[piece][index];
    }
}

void loadFENStr(Board *board, char *FENCode) {
    int rank = 7;
    int file = 0;
    for(int i = 0; i < 64; i ++) {
        board->pieceCodes[i] = EMPTY;
    }
    for(int i = 0; i < 6; i ++) {
        board->pieces[i][0] = 0;
        board->pieces[i][1] = 0;
    }
    
    int i = 0;
    
    board->gameState = 0;
    board->bitboard = 0;
    board->colorBitboards[0] = 0;
    board->colorBitboards[1] = 0;

    while(FENCode[i] != ' ') {
        if(FENCode[i] == '/') {
            rank --;
            file = 0;
        } else if(FENCode[i] > 64) {
            unsigned char pieceByte = getPieceByte(FENCode[i]);
            setIndex(board, rank * 8 + file, pieceByte);
            file ++;
        } else {
            file += FENCode[i] - 48;
        }
        i++;
    }
    i++;
    i += 2;

    while(FENCode[i] != ' ') {
        if(FENCode[i] == 'Q') {
            board->gameState |= WHITE_CASTLE_QUEENSIDE_RIGHT;
        } else if(FENCode[i] == 'q') {
            board->gameState |= BLACK_CASTLE_QUEENSIDE_RIGHT;
        } else if(FENCode[i] == 'K') {
            board->gameState |= WHITE_CASTLE_KINGSIDE_RIGHT;
        } else if(FENCode[i] == 'k') {
            board->gameState |= BLACK_CASTLE_KINGSIDE_RIGHT;
        }
        i++;
    }
    i++;

    if(FENCode[i] != '-') {
        G_SET_EP_FILE(FENCode[i] - 97);
        i++;
    }
    i+= 2;
    int len = 0;
    while(FENCode[i] != ' ') {
        i ++;
        len ++;
    }
    char halfMovesChar[len];
    memcpy(halfMovesChar, &FENCode[i-len], len);
    board->gameState |= atoi(halfMovesChar) << 8;

    z_getKey(board, &board->zobrist);
}

int getFile(char file) {
    return file - 97;
}

int getRank(char rank) {
    return rank - 49;
}

void getSquareStr(int square, char *str) {
    int rank = (int)(square / 8);
    int file = square % 8;

    str[0] = file + 97;
    str[1] = rank + 49;
    str[2] = '\0';
}

int getSquareFromStr(char *square) {
    int file = square[0] - 97;
    int rank = square[1] - 49;
    return rank*8 + file;
}