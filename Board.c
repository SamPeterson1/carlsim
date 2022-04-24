#include "Board.h"

Board g_board;
int flipped = FALSE;

void flipView(void) {
    flipped = 1 - flipped;
}

void printBoard(void) {    
    printf("\e[1;1H\e[2J");
    if(flipped) {
        for(int rank = 0; rank < 8; rank ++) {
            printf("    +---+---+---+---+---+---+---+---+\n");
            printf("(%d) ", rank+1);
            for(int file = 7; file >= 0; file --) {
                printf("| %c ", getPieceChar(g_board.pieceCodes[rank * 8 + file]));
            }
            printf("|\n");
        }
        printf("    +---+---+---+---+---+---+---+---+\n");
        printf("     (h) (g) (f) (e) (d) (c) (b) (a)\n\n");
    } else {
        for(int rank = 7; rank >= 0; rank --) {
            printf("    +---+---+---+---+---+---+---+---+\n");
            printf("(%d) ", rank+1);
            for(int file = 0; file < 8; file ++) {
                printf("| %c ", getPieceChar(g_board.pieceCodes[rank * 8 + file]));
            }
            printf("|\n");
        }
        printf("    +---+---+---+---+---+---+---+---+\n");
        printf("     (a) (b) (c) (d) (e) (f) (g) (h)\n\n");
    }
    
    
}

unsigned long micros(void) {
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return 1000000 * tv.tv_sec + tv.tv_usec;
}

void printInfo(void) {
    printf((TURN == WHITE) ? "\nWhite to move\n" : "\nBlack to move\n");
    printf("BlackKCastleRight: %d \n", HAS_CASTLE_RIGHT(BLACK_CASTLE_KINGSIDE_RIGHT));
    printf("BlackQCastleRight: %d \n", HAS_CASTLE_RIGHT(BLACK_CASTLE_QUEENSIDE_RIGHT));
    printf("WhiteKCastleRight: %d \n", HAS_CASTLE_RIGHT(WHITE_CASTLE_KINGSIDE_RIGHT));
    printf("WhiteQCastleRight: %d \n", HAS_CASTLE_RIGHT(WHITE_CASTLE_QUEENSIDE_RIGHT));
    printf("En Passant File: %d \n", EP_FILE);
    printf("Half Move Clock: %d \n", HALFMOVE_COUNTER);
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

void clearIndex(int index) {
    unsigned char piece = g_board.pieceCodes[index];
    g_board.bitboard &= ~(1ULL << index);
    g_board.pieces[PIECE_TYPE(piece) >> 1][PIECE_COLOR(piece)] &= ~(1ULL << index);
    g_board.colorBitboards[PIECE_COLOR(piece)] &= ~(1ULL << index);

    if(g_board.pieceCodes[index] != EMPTY) {
        g_board.zobrist ^= g_zPieceSquareKeys[piece][index];
    }
    g_board.pieceCodes[index] = EMPTY;
}

void setIndex( int index, unsigned char piece) {
    clearIndex(index);
    if(piece != EMPTY) {
        g_board.bitboard |= (1ULL << index);
        g_board.pieces[PIECE_TYPE(piece) >> 1][PIECE_COLOR(piece)] |= (1ULL << index);
        g_board.colorBitboards[PIECE_COLOR(piece)] |= (1ULL << index);
        g_board.pieceCodes[index] = piece;
        g_board.zobrist ^= g_zPieceSquareKeys[piece][index];
    }
}

void getFENStr(char *FENCode) {
    int ptr = 0;
    for(int rank = 7; rank >= 0; rank --) {
        int file = 0;
        while(file < 8) {
            int skipped = 0;
            while(file < 8 && g_board.pieceCodes[rank*8 + file] == EMPTY) {
                skipped ++;
                file ++;
            }

            if(skipped != 0) FENCode[ptr++] = skipped + 48;
            unsigned char piece = g_board.pieceCodes[rank*8 + file];
            if(file < 8 && piece != EMPTY) {
                FENCode[ptr++] = getPieceChar(piece);
            }
            file ++;
        }
        if(rank != 0) FENCode[ptr++] = '/';
    }
    FENCode[ptr ++] = ' ';

    if(TURN == WHITE) FENCode[ptr++] = 'w';
    else FENCode[ptr++] = 'b';
    FENCode[ptr ++] = ' ';

    int start = ptr;
    if(HAS_CASTLE_RIGHT(WHITE_CASTLE_KINGSIDE_RIGHT)) FENCode[ptr++] = 'K';
    if(HAS_CASTLE_RIGHT(WHITE_CASTLE_QUEENSIDE_RIGHT)) FENCode[ptr++] = 'Q';
    if(HAS_CASTLE_RIGHT(BLACK_CASTLE_KINGSIDE_RIGHT)) FENCode[ptr++] = 'k';
    if(HAS_CASTLE_RIGHT(BLACK_CASTLE_QUEENSIDE_RIGHT)) FENCode[ptr++] = 'q';
    if(start == ptr) FENCode[ptr++] = '-';
    FENCode[ptr ++] = ' ';

    int epFile = EP_FILE;
    if(epFile != -1) {
        FENCode[ptr++] = epFile + 97;
        FENCode[ptr++] = (TURN == WHITE) ? '3' : '6';
    } else {
        FENCode[ptr++] = '-';
    }
    FENCode[ptr ++] = ' ';

    int halfMoveCounter = HALFMOVE_COUNTER;
    int placeVal = 1;
    char buff[5];
    int buffLen = 0;

    while(halfMoveCounter) {
        int digit = (halfMoveCounter / placeVal) % 10;
        halfMoveCounter -= digit * placeVal;
        placeVal *= 10;
        buff[buffLen++] = digit + 48;
    }

    for(int i = buffLen-1; i >= 0; i --) {
        FENCode[ptr++] = buff[i];
    }

    FENCode[ptr++] = ' ';
    FENCode[ptr++] = '0';
    FENCode[ptr] = '\0';
}

void loadFENStr(char *FENCode) {
    int rank = 7;
    int file = 0;
    for(int i = 0; i < 64; i ++) {
        g_board.pieceCodes[i] = EMPTY;
    }
    for(int i = 0; i < 6; i ++) {
        g_board.pieces[i][0] = 0;
        g_board.pieces[i][1] = 0;
    }
    
    int i = 0;
    
    g_board.gameState = 0;
    g_board.bitboard = 0;
    g_board.colorBitboards[0] = 0;
    g_board.colorBitboards[1] = 0;

    while(FENCode[i] != ' ') {
        if(FENCode[i] == '/') {
            rank --;
            file = 0;
        } else if(FENCode[i] > 64) {
            unsigned char pieceByte = getPieceByte(FENCode[i]);
            setIndex(rank * 8 + file, pieceByte);
            file ++;
        } else {
            file += FENCode[i] - 48;
        }
        i++;
    }
    i++;

    if(FENCode[i] == 'w') SET_TURN(WHITE);
    else SET_TURN(BLACK);
    i+= 2;

    while(FENCode[i] != ' ') {
        if(FENCode[i] == 'Q') {
            g_board.gameState |= WHITE_CASTLE_QUEENSIDE_RIGHT;
        } else if(FENCode[i] == 'q') {
            g_board.gameState |= BLACK_CASTLE_QUEENSIDE_RIGHT;
        } else if(FENCode[i] == 'K') {
            g_board.gameState |= WHITE_CASTLE_KINGSIDE_RIGHT;
        } else if(FENCode[i] == 'k') {
            g_board.gameState |= BLACK_CASTLE_KINGSIDE_RIGHT;
        }
        i++;
    }
    i++;

    if(FENCode[i] != '-') {
        SET_EP_FILE(FENCode[i] - 97);
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
    SET_HALFMOVE_COUNTER(atoi(halfMovesChar));
    z_getKey(&g_board.zobrist);
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