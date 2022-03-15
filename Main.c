#include <stdio.h>
#include <ctype.h>
#include <string.h> 
#include <stdlib.h>
#include <sys/time.h>

#define STARTING_POS "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 0"
#define TRUE 1
#define FALSE 0
#define MAX_ARG_COUNT 16
#define MAX_ARG_LENGTH 64
#define NO_CASTLE 0
#define WHITE_KING_SIDE_CASTLE 1
#define WHITE_QUEEN_SIDE_CASTLE 2
#define BLACK_KING_SIDE_CASTLE 3
#define BLACK_QUEEN_SIDE_CASTLE 4
#define NO_PROMOTION ' '
#define EMPTY 0
#define PAWN 1
#define KNIGHT 2
#define BISHOP 4
#define ROOK 8
#define QUEEN 16
#define KING 32
#define WHITE 64
#define BLACK 128
#define min(a, b) ((a > b) ? b : a)
#define color(a) (a & 0xC0)
#define piece(a) (a & 0x3F)
#define inv(turn) ((turn == WHITE) ? BLACK : WHITE)
#define get(rank, file) board.board[rank*8+file]
#define set(rank, file, piece)board.board[rank*8+file] = piece
#define index(rank, file) ((rank)*8 + file)

int print = TRUE;

typedef struct PosNode_s {

    struct PosNode_s *next;
    int pos;

} PosNode;

typedef struct Positions_s {

    int len;
    PosNode *head;
    PosNode *tail;

} Positions;

typedef struct MoveNode_s {

    struct MoveNode_s *next;
    int origin;
    int dest;
    int enPassantTarget;
    int castle;
    unsigned char promotion;

} MoveNode;

typedef struct Moves_s {

    int len;
    MoveNode *head;
    MoveNode *tail;

} Moves;

Moves blah;

typedef struct PinNode_s {
    struct PinNode_s *next;
    Positions* pinLine;
    int pieceIndex; 
    int enPassantOnly;
} PinNode;

typedef struct Pins_s {
    int len;
    PinNode *head;
    PinNode *tail;
} Pins;

typedef struct Board_s {
    unsigned char board[64];
    unsigned char turn;

    int enPassantTarget;
    int halfMoves;
    int fullMoves;

    int blackQCastleRight;
    int blackKCastleRight;
    int whiteQCastleRight;
    int whiteKCastleRight;

    Positions whitePawns;
    Positions whiteKnights;
    Positions whiteBishops;
    Positions whiteRooks;
    Positions whiteQueens;
    int whiteKing;

    Positions blackPawns;
    Positions blackKnights;
    Positions blackBishops;
    Positions blackRooks;
    Positions blackQueens;
    int blackKing;

    Pins pins;
    Pins checkers;
    Positions attackedSquares;

} Board;

Board board;

MoveNode lastMove;
int prevEnPassantTarget;
unsigned char prevCapturedPiece;

int prevBlackKCastleRight;
int prevBlackQCastleRight;
int prevWhiteKCastleRight;
int prevWhiteQCastleRight;

int moveCount;

int memCount = 0;

void *fred(int members, int size) {
    memCount ++;
    return calloc(members, size);
}

void *bill(int size) {
    memCount ++;
    return malloc(size);
}

void keel(void *ptr) {
    memCount--;
    free(ptr);
}

int inBounds(int rank, int file) {
    return rank >= 0 && rank < 8 && file >= 0 && file < 8;
}

unsigned long micros() {
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return 1000000 * tv.tv_sec + tv.tv_usec;
}

char getPieceChar(unsigned char pieceByte) {
    char c = ' ';
    switch(piece(pieceByte)) {
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
    if(color(pieceByte) == WHITE) c -= 32;
    return c;
}

void printBoard() {    
    printf("\e[1;1H\e[2J");
    for(int rank = 7; rank >= 0; rank --) {
        printf("    +---+---+---+---+---+---+---+---+\n");
        printf("(%d) ", rank+1);
        for(int file = 0; file < 8; file ++) {
            printf("| %c ", getPieceChar(get(rank, file)));
        }
        printf("|\n");
    }
    printf("    +---+---+---+---+---+---+---+---+\n");
    printf("     (a) (b) (c) (d) (e) (f) (g) (h)\n\n");
}

int parseSquare(char *square) {
    int rank = square[1] - 49;
    int file = square[0] - 97;

    return rank*8 + file;
}

void getSquareChar(int index, char*square) {
    int rank = index / 8;
    int file = index % 8;

    square[0] = (char) file + 97;
    square[1] = (char) rank + 49;
    square[2] = (char) 0;
}

void addPin(Pins *pins, PinNode *node) {
    if(pins->head == NULL) {
        pins->head = node;
        pins->tail = node;
    } else {
        pins->tail->next = node;
        pins->tail = node;
    }
    pins->len ++;
}

void addMoveNode(Moves *moves, MoveNode *node) {
    if(moves->head == NULL) {
        moves->head = node;
        moves->tail = node;
    } else {
        moves->tail->next = node;
        moves->tail = node;
    }
    moves->len ++;
}

MoveNode *allocDefaultMoveNode() {
    MoveNode *node;
    node = (MoveNode *)fred(1, sizeof(MoveNode));

    node->origin = -1;
    node->dest = -1;
    node->enPassantTarget = -1;
    node->castle = NO_CASTLE;
    node->promotion = NO_PROMOTION;

    return node;
}

void addMove(Moves *moves, int origin, int dest) {
    if(origin >= 0 && origin < 64 && dest >= 0 && dest < 64) {
        MoveNode *node = allocDefaultMoveNode();

        node->origin = origin;
        node->dest = dest;
        addMoveNode(moves, node);
    }
}

int containsMove(Moves moves, MoveNode move) {
    MoveNode* current = moves.head;
    while(current != NULL) {
        if(current->promotion == move.promotion && current->castle == move.castle && current->dest == move.dest && current->enPassantTarget == move.enPassantTarget && current->origin == move.origin)  {
            return TRUE;
        }
        current = current->next;
    }
    return FALSE;
}

void addPromotionMove(Moves *moves, int origin, int dest, unsigned char promotion) {
    MoveNode *node = allocDefaultMoveNode();

    node->origin = origin;
    node->dest = dest;
    node->promotion = promotion;

    addMoveNode(moves, node);
}

void addCastleMove(Moves *moves, int castle) {
    MoveNode *node = allocDefaultMoveNode();

    node->castle = castle;
    addMoveNode(moves, node);
}

void printMoves(Moves moves) {
    MoveNode *current = moves.head;
    printf("Len: %d\n", moves.len);
    while(current != NULL) {
        char origin[3];
        char dest[3];
        getSquareChar(current->origin, origin);
        getSquareChar(current->dest, dest);
        if(current->castle == NO_CASTLE) {
            printf("%s %s %c \n", origin, dest, current->promotion);
        } else if(current->castle == WHITE_QUEEN_SIDE_CASTLE || current->castle == BLACK_QUEEN_SIDE_CASTLE) {
            printf("O-O-O %d\n", current->castle);
        } else if(current->castle == WHITE_KING_SIDE_CASTLE || current->castle == BLACK_KING_SIDE_CASTLE) {
            printf("O-O %d\n", current->castle);
        }
        current = current->next;
    }
}

void addPosition(Positions *positions, int pos) {
    PosNode *node;
    node = (PosNode *)fred(1, sizeof(PosNode));
    node->pos = pos;
    if(positions->head == NULL) {
        positions->head = node;
        positions->tail = node;
    } else {
        positions->tail->next = node;
        positions->tail = node;
    }
    positions->len ++;
}

void clearMoves(Moves *moves) {
    MoveNode *current = moves->head;
    while(current != NULL) {   
        MoveNode *next = current->next;
        keel(current);
        current = next;
    }
    moves->len = 0;
    moves->head = NULL;
    moves->tail = NULL;
}


void clearPositions(Positions *positions) {
    PosNode *current = positions->head;
    while(current != NULL) {   
        PosNode *next = current->next;
        keel(current);
        current = next;
    }
    positions->len = 0;
    positions->head = NULL;
    positions->tail = NULL;
}

void clearPins(Pins *pins) {
    PinNode *current = pins->head;
    while(current != NULL) {   
        PinNode *next = current->next;
        if(current->pinLine != NULL) clearPositions(current->pinLine);
        keel(current);
        current = next;
    }
    pins->len = 0;
    pins->head = NULL;
    pins->tail = NULL;
}

void removePosition(Positions *positions, int pos) {
    PosNode *current = positions->head;
    PosNode *last = NULL;
    
    while(current != NULL) {
        if(current->pos == pos) {
            positions->len --;
            if(last == NULL) {
                positions->head = current->next;
            } else {
                last->next = current->next;
                if(last->next == NULL) positions->tail = last;
            }
            keel(current);
            break;
        }
        last = current;
        current = current->next;
    }
}

char **init()
{

    char **ret;

    ret = (char **)fred(1, MAX_ARG_COUNT * sizeof(char *));
    *(ret + 0) = (char *)fred(1, MAX_ARG_COUNT * (MAX_ARG_LENGTH + 1) * sizeof(char));
    for (int i = 0; i < MAX_ARG_COUNT; i++)
    {
        *(ret + i) = (*ret + (MAX_ARG_LENGTH + 1) * i);
    }

    return ret;
}

void parse(const char *str, char **args, int *argc)
{
    int pos = 0;
    int i = 0;
    int ch = 0;
    int len = 0;
    int intoken = TRUE;
    int inliteral = FALSE;

    *argc = 0;
    memset(*(args + 0), 0, MAX_ARG_COUNT * (MAX_ARG_LENGTH + 1) * sizeof(char));

    len = strlen(str);

    for (i = 0; i < len; i++)
    {
        ch = str[i];
        if (ch == 32)
        {
            if (inliteral)
            {
                *(args[*argc] + pos) = ch;
                pos += 1;
            }
            else
            {
                if (intoken)
                {
                    intoken = FALSE;
                    inliteral = FALSE;
                    pos = 0;
                }
            }
        }
        else if (ch == 34)
        {
            if (inliteral)
            {
                intoken = FALSE;
                inliteral = FALSE;
                pos = 0;
            }
            else
            {
                intoken = TRUE;
                inliteral = TRUE;
                *argc += 1;
                pos = 0;
            }
        }
        else if (ch >= 33 && ch <= 126)
        {

            if (intoken)
            {
                *(args[*argc] + pos) = ch;
                pos += 1;
            }
            else
            {
                *argc += 1;
                pos = 0;
                *(args[*argc] + pos) = ch;
                pos += 1;
            }
            intoken = TRUE;
        }
    }

    *argc += 1;
}

void loadFENStr(char *FENCode) {
    int rank = 7;
    int file = 0;
    int i = 0;

    for(int rank = 0; rank < 8; rank ++) {
        for(int file = 0; file < 8; file ++) {
            set(rank, file, EMPTY);
        }
    }

    
    clearPositions(&board.blackPawns);
    clearPositions(&board.blackKnights);
    clearPositions(&board.blackBishops);
    clearPositions(&board.blackRooks);
    clearPositions(&board.blackQueens);

    clearPositions(&board.whitePawns);
    clearPositions(&board.whiteKnights);
    clearPositions(&board.whiteBishops);
    clearPositions(&board.whiteRooks);
    clearPositions(&board.whiteQueens);
    

    while(FENCode[i] != ' ') {
        if(FENCode[i] == '/') {
            rank --;
            file = 0;
        } else if(FENCode[i] > 64) {

            switch(FENCode[i]) {
                case 'p':
                    set(rank, file, BLACK | PAWN);
                    addPosition(&board.blackPawns, rank*8 + file);
                    break;
                case 'n':
                    set(rank, file, BLACK | KNIGHT);
                    addPosition(&board.blackKnights, rank*8 + file);
                    break;
                case 'b':
                    set(rank, file, BLACK | BISHOP);
                    addPosition(&board.blackBishops, rank*8 + file);
                    break;
                case 'r':
                    set(rank, file, BLACK | ROOK);
                    addPosition(&board.blackRooks, rank*8 + file);
                    break;
                case 'q':
                    set(rank, file, BLACK | QUEEN);
                    addPosition(&board.blackQueens, rank*8 + file);
                    break;
                case 'k':
                    set(rank, file, BLACK | KING);
                    board.blackKing = rank*8 + file;
                    break;
                case 'P':
                    set(rank, file, WHITE | PAWN);
                    addPosition(&board.whitePawns, rank*8 + file);
                    break;
                case 'N':
                    set(rank, file, WHITE | KNIGHT);
                    addPosition(&board.whiteKnights, rank*8 + file);
                    break;
                case 'B':
                    set(rank, file, WHITE | BISHOP);
                    addPosition(&board.whiteBishops, rank*8 + file);
                    break;
                case 'R':
                    set(rank, file, WHITE | ROOK);
                    addPosition(&board.whiteRooks, rank*8 + file);
                    break;
                case 'Q':
                    set(rank, file, WHITE | QUEEN);
                    addPosition(&board.whiteQueens, rank*8 + file);
                    break;
                case 'K':
                    set(rank, file, WHITE | KING);
                    board.whiteKing = rank*8 + file;
                    break;
            }
            file ++;
        } else {
            file += FENCode[i] - 48;
        }
        i++;
    }
    i++;
    board.turn = (FENCode[i] == 'w') ? WHITE : BLACK;
    i += 2;

    board.blackKCastleRight = FALSE;
    board.blackQCastleRight = FALSE;
    board.whiteKCastleRight = FALSE;
    board.whiteQCastleRight = FALSE;

    while(FENCode[i] != ' ') {
        if(FENCode[i] == 'Q') {
            board.whiteQCastleRight = TRUE;
        } else if(FENCode[i] == 'q') {
            board.blackQCastleRight = TRUE;
        } else if(FENCode[i] == 'K') {
            board.whiteKCastleRight = TRUE;
        } else if(FENCode[i] == 'k') {
            board.blackKCastleRight = TRUE;
        }
        i++;
    }
    i++;

    board.enPassantTarget = -1;
    if(FENCode[i] != '-') {
        board.enPassantTarget = (FENCode[i+1] - 49)*8 + (FENCode[i] - 97);
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
    board.halfMoves = atoi(halfMovesChar);

    len = 0;
    while(FENCode[i] != '\0') {
        i ++;
        len ++;
    }
    char fullMovesChar[len];
    memcpy(fullMovesChar, &FENCode[i-len], len);
    board.fullMoves = atoi(fullMovesChar);
}

Positions *getPositionsOfType(unsigned char piece) {
    switch(piece) {
        case (BLACK | PAWN):
            return &board.blackPawns;
        case (BLACK | KNIGHT):
            return &board.blackKnights;
        case (BLACK | BISHOP):
            return &board.blackBishops;
        case (BLACK | ROOK):
            return &board.blackRooks;
        case (BLACK | QUEEN):
            return &board.blackQueens;
        case (WHITE | PAWN):
            return &board.whitePawns;
        case (WHITE | KNIGHT):
            return &board.whiteKnights;
        case (WHITE | BISHOP):
            return &board.whiteBishops;
        case (WHITE | ROOK):
            return &board.whiteRooks;
        case (WHITE | QUEEN):
            return &board.whiteQueens;
    }

    return NULL;
}

void printPositions(Positions positions) {
    PosNode *current = positions.head;
    while(current != NULL) {
        char pos[3];
        getSquareChar(current->pos, pos);
        printf("%s ", pos);
        current = current->next;
    }
    printf("\n");
}

void getDebugInfo() {
    printBoard();
    printf((board.turn == WHITE) ? "\nWhite to move\n" : "\nBlack to move\n");
    printf("%d\n", board.turn);
    printf("BlackKCastleRight: %d \n", board.blackKCastleRight);
    printf("BlackQCastleRight: %d \n", board.blackQCastleRight);
    printf("WhiteKCastleRight: %d \n", board.whiteKCastleRight);
    printf("WhiteQCastleRight: %d \n", board.whiteQCastleRight);
    printf("En Passant Target: %d \n", board.enPassantTarget);
    printf("Half Moves Completed: %d \n", board.halfMoves);
    printf("Full Move Clock: %d \n\n", board.fullMoves);   

    printf("White \n");
    printf("Pawns: "); printPositions(board.whitePawns);
    printf("Knights: "); printPositions(board.whiteKnights);
    printf("Bishops: "); printPositions(board.whiteBishops);
    printf("Rooks: "); printPositions(board.whiteRooks);
    printf("Queens: "); printPositions(board.whiteQueens);
    printf("King: %d\n", board.whiteKing);

    printf("Black \n");
    printf("Pawns: "); printPositions(board.blackPawns);
    printf("Knights: "); printPositions(board.blackKnights);
    printf("Bishops: "); printPositions(board.blackBishops);
    printf("Rooks: "); printPositions(board.blackRooks);
    printf("Queens: "); printPositions(board.blackQueens);
    printf("King: %d\n", board.blackKing);
}

void unMakeMove(MoveNode move, unsigned char capturedPiece, int lastEnPassantTarget) {
    
    if(move.enPassantTarget != -1) {
        board.board[move.enPassantTarget] = capturedPiece;
        Positions *capturedPosition = getPositionsOfType(capturedPiece);
        addPosition(capturedPosition, move.enPassantTarget);

        Positions *movedPosition = getPositionsOfType(board.board[move.dest]);
        removePosition(movedPosition, move.dest);
        addPosition(movedPosition, move.origin);
        board.board[move.origin] = board.board[move.dest];
        board.board[move.dest] = EMPTY;
    } else if(move.castle != NO_CASTLE) {
        if(move.castle == WHITE_KING_SIDE_CASTLE) {
            board.board[7] = WHITE | ROOK;
            board.board[4] = WHITE | KING;
            board.board[6] = EMPTY;
            board.board[5] = EMPTY;
            board.whiteKing = 4;
            addPosition(&board.whiteRooks, 7);
            removePosition(&board.whiteRooks, 5);
        } else if(move.castle == WHITE_QUEEN_SIDE_CASTLE) {
            board.board[0] = WHITE | ROOK;
            board.board[4] = WHITE | KING;
            board.board[2] = EMPTY;
            board.board[3] = EMPTY;
            board.whiteKing = 4;
            addPosition(&board.whiteRooks, 0);
            removePosition(&board.whiteRooks, 3);
        } else if(move.castle == BLACK_KING_SIDE_CASTLE) {
            board.board[63] = BLACK | ROOK;
            board.board[60] = BLACK | KING;
            board.board[62] = EMPTY;
            board.board[61] = EMPTY;
            board.blackKing = 60;
            addPosition(&board.blackRooks, 63);
            removePosition(&board.blackRooks, 61);
        } else if(move.castle == BLACK_QUEEN_SIDE_CASTLE) {
            board.board[56] = BLACK | ROOK;
            board.board[60] = BLACK | KING;
            board.board[58] = EMPTY;
            board.board[59] = EMPTY;
            board.blackKing = 60;
            addPosition(&board.blackRooks, 56);
            removePosition(&board.blackRooks, 59);
        }
    } else {
        if(capturedPiece == (WHITE | KING)) {
            board.whiteKing = move.dest;
            board.board[move.dest] = WHITE | KING;
        } else if(capturedPiece == (BLACK | KING)) {
            board.board[move.dest] = BLACK | KING;
            board.blackKing = move.dest;
        } else if(capturedPiece != EMPTY) {
            Positions *capturedPosition = getPositionsOfType(capturedPiece);
            addPosition(capturedPosition, move.dest);
        }
        if(board.board[move.dest] == (WHITE | KING)) {
           board.whiteKing = move.origin;
           board.board[move.origin] = WHITE | KING;
           board.board[move.dest] = capturedPiece;
       } else if(board.board[move.dest] == (BLACK | KING)) {
           board.blackKing = move.origin;
           board.board[move.origin] = BLACK | KING;
           board.board[move.dest] = capturedPiece;
       } else {
            Positions *movedPosition = getPositionsOfType(board.board[move.dest]);
            removePosition(movedPosition, move.dest);
            addPosition(movedPosition, move.origin);

            board.board[move.origin] = board.board[move.dest];
            board.board[move.dest] = capturedPiece;
       }
    }

    if(move.promotion != NO_PROMOTION) {
        Positions *pawnPosition = (board.turn == WHITE) ? &board.blackPawns : &board.whitePawns;
        Positions *promotedPosition = getPositionsOfType(move.promotion);
        removePosition(promotedPosition, move.origin);
        addPosition(pawnPosition, move.origin);
        board.board[move.origin] = PAWN | inv(board.turn);
    }

    board.enPassantTarget = lastEnPassantTarget;
    
    board.turn = inv(board.turn);
}

unsigned char makeMove(MoveNode move) {
    unsigned char capture = EMPTY;
    board.enPassantTarget = -1;
    
    if(move.origin == 0 || move.dest == 0) {
        board.whiteQCastleRight = FALSE;
    } else if(move.origin == 7 || move.dest == 7) {
        board.whiteKCastleRight = FALSE;
    } else if(move.origin == 63 || move.dest == 63) {
        board.blackKCastleRight = FALSE;
    } else if(move.origin == 56 || move.dest == 56) {
        board.blackQCastleRight = FALSE;
    }

    if(move.enPassantTarget != -1) {
       Positions *movedPosition = getPositionsOfType(board.board[move.origin]);
       Positions *capturedPosition = getPositionsOfType(board.board[move.enPassantTarget]);
       removePosition(capturedPosition, move.enPassantTarget);
       removePosition(movedPosition, move.origin);
       addPosition(movedPosition, move.dest);

       board.board[move.dest] = board.board[move.origin];
       board.board[move.origin] = EMPTY;
       capture = board.board[move.enPassantTarget];
       board.board[move.enPassantTarget] = EMPTY;
    } else if(move.castle != NO_CASTLE) {
        if(move.castle == WHITE_KING_SIDE_CASTLE) {
            board.board[7] = EMPTY;
            board.board[4] = EMPTY;
            board.board[6] = WHITE | KING;
            board.board[5] = WHITE | ROOK;
            board.whiteKing = 6;
            board.whiteKCastleRight = FALSE;
            board.whiteQCastleRight = FALSE;
            removePosition(&board.whiteRooks, 7);
            addPosition(&board.whiteRooks, 5);
        } else if(move.castle == WHITE_QUEEN_SIDE_CASTLE) {
            board.board[0] = EMPTY;
            board.board[4] = EMPTY;
            board.board[2] = WHITE | KING;
            board.board[3] = WHITE | ROOK;
            board.whiteKing = 2;
            board.whiteKCastleRight = FALSE;
            board.whiteQCastleRight = FALSE;
            removePosition(&board.whiteRooks, 0);
            addPosition(&board.whiteRooks, 3);
        } else if(move.castle == BLACK_KING_SIDE_CASTLE) {
            board.board[63] = EMPTY;
            board.board[60] = EMPTY;
            board.board[62] = BLACK | KING;
            board.board[61] = BLACK | ROOK;
            board.blackKing = 62;
            board.blackKCastleRight = FALSE;
            board.blackQCastleRight = FALSE;
            removePosition(&board.blackRooks, 63);
            addPosition(&board.blackRooks, 61);
        } else if(move.castle == BLACK_QUEEN_SIDE_CASTLE) {
            board.board[56] = EMPTY;
            board.board[60] = EMPTY;
            board.board[58] = BLACK | KING;
            board.board[59] = BLACK | ROOK;
            board.blackKing = 58;
            board.blackKCastleRight = FALSE;
            board.blackQCastleRight = FALSE;
            removePosition(&board.blackRooks, 56);
            addPosition(&board.blackRooks, 59);
        }
    } else {
        if(board.board[move.dest] != EMPTY && board.board[move.dest] != (WHITE | KING) && board.board[move.dest] != (BLACK | KING)) {
            Positions *capturedPosition = getPositionsOfType(board.board[move.dest]);
            removePosition(capturedPosition, move.dest);
            capture = board.board[move.dest];
        } else {
            capture = board.board[move.dest];
        }
        if(board.board[move.origin] == (WHITE | KING)) {
            board.whiteKCastleRight = FALSE;
            board.whiteQCastleRight = FALSE;
            board.whiteKing = move.dest;
            capture = board.board[move.dest];
            board.board[move.dest] = WHITE | KING;
            board.board[move.origin] = EMPTY;  
        } else if(board.board[move.origin] == (BLACK | KING)) {
            board.blackKCastleRight = FALSE;
            board.blackQCastleRight = FALSE;
            board.blackKing = move.dest;
            capture = board.board[move.dest];
            board.board[move.dest] = BLACK | KING;
            board.board[move.origin] = EMPTY;
        } else {
            
            Positions *movedPosition = getPositionsOfType(board.board[move.origin]);
            removePosition(movedPosition, move.origin);
            addPosition(movedPosition, move.dest);
            
            board.board[move.dest] = board.board[move.origin];
            board.board[move.origin] = EMPTY;

            if(piece(board.board[move.dest]) == PAWN && abs(move.dest - move.origin) == 16) {
                int epPiece = (PAWN | inv(color(board.board[move.dest])));
                if((move.dest % 8 != 0 && board.board[move.dest-1] == epPiece) || (move.dest % 8 != 7 && board.board[move.dest+1] == epPiece)) {
                    board.enPassantTarget = move.dest;
                }
            }
            
        }
    }

    if(move.promotion != NO_PROMOTION) {
        Positions *pawnPosition = (board.turn == WHITE) ? &board.whitePawns : &board.blackPawns;
        Positions *promotedPosition = getPositionsOfType(move.promotion);
        addPosition(promotedPosition, move.dest);
        removePosition(pawnPosition, move.dest);
        board.board[move.dest] = move.promotion;
    }

    board.turn = inv(board.turn);
    return capture;
}

int split (const char *txt, char delim, char ***tokens)
{
    int *tklen, *t, count = 1;
    char **arr, *p = (char *) txt;

    while (*p != '\0') if (*p++ == delim) count += 1;
    t = tklen = fred (count, sizeof (int));
    for (p = (char *) txt; *p != '\0'; p++) *p == delim ? *t++ : (*t)++;
    *tokens = arr = bill (count * sizeof (char *));
    t = tklen;
    p = *arr++ = fred (*(t++) + 1, sizeof (char *));
    while (*txt != '\0')
    {
        if (*txt == delim)
        {
            p = *arr++ = fred (*(t++) + 1, sizeof (char *));
            txt++;
        }
        else *p++ = *txt++;
    }
    keel (tklen);
    return count;
}

int canTake(unsigned char piece, int colorToMove) {
    if(piece == EMPTY) return FALSE;
    if(piece > 96) return colorToMove == WHITE;
    return colorToMove == BLACK;
}

int canMoveTo(unsigned char piece, int colorToMove) {
    if(piece == EMPTY) return TRUE;
    if(piece > 96) return colorToMove == WHITE;
    return colorToMove == BLACK;
}


void addMove2(Moves *moves, int origin, int destRank, int destFile, int colorToMove) {
    if(destFile >= 0 && destFile < 8 && destRank >= 0 && destRank < 8 && canMoveTo(get(destRank, destFile), colorToMove)) {
        addMove(moves, origin, destRank*8 + destFile);
    }
}

void addMove3(Moves *moves, int origin, int destRank, int destFile, int *pathBlocked, int colorToMove) {
    if(*pathBlocked == FALSE && destFile >= 0 && destFile < 8 && destRank >= 0 && destRank < 8 && canMoveTo(get(destRank, destFile), colorToMove)) {
        addMove(moves, origin, destRank*8 + destFile);
    }
    if(get(destRank, destFile) != EMPTY) {
        *pathBlocked = TRUE;
    }
}

void addEnPassantMove(Moves *moves, int origin, int dest) {
    if(origin >= 0 && origin < 64 && dest >= 0 && dest < 64) {
        MoveNode *node = allocDefaultMoveNode();
        node->origin = origin;
        node->dest = dest;
        node->enPassantTarget = board.enPassantTarget;

        addMoveNode(moves, node);
    }
}

void addMoveRay(Moves *moves, int origin, int dRank, int dFile, int checkDestColor) {
    int rank = origin / 8 + dRank;
    int file = origin % 8 + dFile;
    int colorToMove = color(board.board[origin]);
    while(inBounds(rank, file) && (!checkDestColor || canMoveTo(get(rank, file), colorToMove))) {
        MoveNode *node = allocDefaultMoveNode();
        node->origin = origin;
        node->dest = rank*8 + file;
        addMoveNode(moves, node);
        if(color(get(rank, file)) == inv(colorToMove)) break;
        rank += dRank;
        file += dFile;
    }
}

int containsPosition(Positions positions, int pos) {
    PosNode *current = positions.head;
    while(current != NULL) {
        if(current->pos == pos) {
            return TRUE;
        }
        current = current->next;
    }

    return FALSE;
}

void addPositionNoDuplicates(Positions *attackedSquares, Pins *checkers, int pos, int origin) {
    if(board.board[pos] == (board.turn | KING)) {
        PinNode *node = (PinNode *) bill(sizeof(PinNode));
        node->next = NULL;
        node->pinLine = NULL;
        node->enPassantOnly = FALSE;
        node->pieceIndex = origin;
        addPin(checkers, node);
    }
    if(!containsPosition(*attackedSquares, pos)) {
        addPosition(attackedSquares, pos);
    }
}

void computeRay(Positions *attackedPositions, Pins *checkers, Pins *pins, int origin, int dRank, int dFile) {
    int rank = origin / 8 + dRank;
    int file = origin % 8 + dFile;
    int colorToMove = color(board.board[origin]);
    int pathBlocked = FALSE; //set to true when running into any piece besides king
    int pinExists = FALSE; //set to true when pathBlocked = true and you run into king
    int pinBlocked = FALSE; //set to true when pathBlocked = true and you run into anything
    int checkExists = FALSE; //set to true when pathBlocked = false and you run into king  
    int index = -1; //set to true when you run into piece besides king
    int enPassantOnly = FALSE;
    Positions *pinLine = (Positions *) fred(1, sizeof(Positions));
    Positions *checkLine = (Positions *) fred(1, sizeof(Positions));
    addPosition(checkLine, origin);
    addPosition(pinLine, origin);
    while(inBounds(rank, file)) {
        if(!pathBlocked) {
            if(!enPassantOnly) addPosition(attackedPositions, index(rank, file));
            addPosition(pinLine, index(rank, file));
            if(!checkExists) addPosition(checkLine, index(rank, file));
            if(index(rank, file) == board.enPassantTarget) {
                enPassantOnly = TRUE;
            } else if(get(rank, file) != EMPTY && get(rank, file) != (KING | inv(colorToMove))) {
                pathBlocked = TRUE;
                index = index(rank, file);
            } else if(get(rank, file) == (KING | inv(colorToMove))) {
                index = origin;
                checkExists = TRUE;
            }
        } else if(!pinBlocked) {
            if(index(rank, file) == board.enPassantTarget) {
                enPassantOnly = TRUE;
            } else if(get(rank, file) != EMPTY) {
                pinBlocked = TRUE;
                pinExists = FALSE;
            }
            if(get(rank, file) == (KING | inv(colorToMove))) {
                pinExists = TRUE;
            }
            addPosition(pinLine, index(rank, file));
        }
        rank += dRank;
        file += dFile;
    }

    
    if(pinExists) {
        PinNode *node = bill(sizeof(PinNode));
        node->pinLine = pinLine;
        node->pieceIndex = index;
        node->next = NULL;
        node->enPassantOnly = enPassantOnly;
        addPin(pins, node);
    } else {
        clearPositions(pinLine);
        keel(pinLine);
    }
    if(checkExists) {
        PinNode *node = bill(sizeof(PinNode));
        node->pinLine = checkLine;
        node->pieceIndex = index;
        node->next = NULL;
        node->enPassantOnly = FALSE;
        addPin(checkers, node);
    } else {
        clearPositions(checkLine);
        keel(checkLine);
    }
}

void computeDiagonalRays(Positions *attackedPositions, Pins *checkers, Pins *pins, int origin) {
    computeRay(attackedPositions, checkers, pins, origin, 1, 1);
    computeRay(attackedPositions, checkers, pins, origin, 1, -1);
    computeRay(attackedPositions, checkers, pins, origin, -1, 1);
    computeRay(attackedPositions, checkers, pins, origin, -1, -1);
}

void computeSlidingRays(Positions *attackedPositions, Pins *checkers, Pins *pins, int origin) {
    computeRay(attackedPositions, checkers, pins, origin, 1, 0);
    computeRay(attackedPositions, checkers, pins, origin, 0, 1);
    computeRay(attackedPositions, checkers, pins, origin, -1, 0);
    computeRay(attackedPositions, checkers, pins, origin, 0, -1);
}

void addDiagonalMoves(Moves *moves, int origin, int checkDestColor) {
    addMoveRay(moves, origin, 1, 1, checkDestColor);
    addMoveRay(moves, origin, 1, -1, checkDestColor);
    addMoveRay(moves, origin, -1, 1, checkDestColor);
    addMoveRay(moves, origin, -1, -1, checkDestColor);
}

void addSlidingMoves(Moves *moves, int origin, int checkDestColor) {
    addMoveRay(moves, origin, 1, 0, checkDestColor);
    addMoveRay(moves, origin, 0, 1, checkDestColor);
    addMoveRay(moves, origin, -1, 0, checkDestColor);
    addMoveRay(moves, origin, 0, -1, checkDestColor);
}

void getAttackedSquaresPinsAndCheckers(Positions *attackedSquares, Pins *pins, Pins *checkers) {
    PosNode *current = (board.turn == WHITE) ? board.blackPawns.head : board.whitePawns.head;
    int perspective = (board.turn == WHITE) ? -1 : 1;
    while(current != NULL) {
        if(current->pos%8 != (3.5-perspective*3.5)) {
            addPositionNoDuplicates(attackedSquares, checkers, current->pos + perspective*7, current->pos);
        }
        if(current->pos%8 != (3.5+perspective*3.5)) {
            addPositionNoDuplicates(attackedSquares, checkers, current->pos + perspective*9, current->pos);
        }
        current = current->next;
    }

    current = (board.turn == WHITE) ? board.blackKnights.head : board.whiteKnights.head;
    while(current != NULL) {
        int file = current->pos%8;
        int rank = current->pos/8;
        if(inBounds(rank+1, file+2)) addPositionNoDuplicates(attackedSquares, checkers, index(rank+1, file+2), current->pos);      
        if(inBounds(rank+2, file+1)) addPositionNoDuplicates(attackedSquares, checkers, index(rank+2, file+1), current->pos);
        if(inBounds(rank-1, file+2)) addPositionNoDuplicates(attackedSquares, checkers, index(rank-1, file+2), current->pos);
        if(inBounds(rank-2, file+1)) addPositionNoDuplicates(attackedSquares, checkers, index(rank-2, file+1), current->pos);
        if(inBounds(rank+1, file-2)) addPositionNoDuplicates(attackedSquares, checkers, index(rank+1, file-2), current->pos);
        if(inBounds(rank+2, file-1)) addPositionNoDuplicates(attackedSquares, checkers, index(rank+2, file-1), current->pos);
        if(inBounds(rank-1, file-2)) addPositionNoDuplicates(attackedSquares, checkers, index(rank-1, file-2), current->pos);
        if(inBounds(rank-2, file-1)) addPositionNoDuplicates(attackedSquares, checkers, index(rank-2, file-1), current->pos);
        current = current->next;
    }

    current = (board.turn == WHITE) ? board.blackBishops.head : board.whiteBishops.head;
    while(current != NULL) {
        computeDiagonalRays(attackedSquares, checkers, pins, current->pos);
        current = current->next;
    }

    current = (board.turn == WHITE) ? board.blackRooks.head : board.whiteRooks.head;
    while(current != NULL) {
        computeSlidingRays(attackedSquares, checkers, pins, current->pos);
        current = current->next;
    }
    
    current = (board.turn == WHITE) ? board.blackQueens.head : board.whiteQueens.head;
    while(current != NULL) {
        computeDiagonalRays(attackedSquares, checkers, pins, current->pos);
        computeSlidingRays(attackedSquares, checkers, pins, current->pos);
        current = current->next;
    }

    int kingPos = (board.turn == WHITE) ? board.blackKing : board.whiteKing;
    int kingRank = kingPos/8;
    int kingFile = kingPos%8;

    if(inBounds(kingRank+1, kingFile+1)) addPositionNoDuplicates(attackedSquares, checkers, index(kingRank + 1, kingFile + 1), kingPos);
    if(inBounds(kingRank+1, kingFile+0)) addPositionNoDuplicates(attackedSquares, checkers, index(kingRank + 1, kingFile + 0), kingPos);
    if(inBounds(kingRank+1, kingFile-1)) addPositionNoDuplicates(attackedSquares, checkers, index(kingRank + 1, kingFile - 1), kingPos);
    if(inBounds(kingRank+0, kingFile+1)) addPositionNoDuplicates(attackedSquares, checkers, index(kingRank + 0, kingFile + 1), kingPos);
    if(inBounds(kingRank+0, kingFile-1)) addPositionNoDuplicates(attackedSquares, checkers, index(kingRank + 0, kingFile - 1), kingPos);
    if(inBounds(kingRank-1, kingFile+1)) addPositionNoDuplicates(attackedSquares, checkers, index(kingRank - 1, kingFile + 1), kingPos);
    if(inBounds(kingRank-1, kingFile+0)) addPositionNoDuplicates(attackedSquares, checkers, index(kingRank - 1, kingFile + 0), kingPos);
    if(inBounds(kingRank-1, kingFile-1)) addPositionNoDuplicates(attackedSquares, checkers, index(kingRank - 1, kingFile - 1), kingPos);
}

void addPawnMove(Moves *moves, int origin, int dest, int colorToMove) {
    if((colorToMove == WHITE && dest/8 == 7) || (colorToMove == BLACK && dest/8 == 0)) {
        addPromotionMove(moves, origin, dest, colorToMove | QUEEN);
        addPromotionMove(moves, origin, dest, colorToMove | ROOK);
        addPromotionMove(moves, origin, dest, colorToMove | KNIGHT);
        addPromotionMove(moves, origin, dest, colorToMove | BISHOP);
    } else {
        addMove(moves, origin, dest);
    }
}

Moves getPseudoLegalMoves(int colorToMove) {

    Moves moves = {
        .len = 0,
        .head = NULL,
        .tail = NULL
    };


    
    //castling
    if(colorToMove == WHITE) {
        if(board.whiteKCastleRight && get(0, 5) == EMPTY && get(0, 6) == EMPTY) {
            addCastleMove(&moves, WHITE_KING_SIDE_CASTLE);
        }
        if(board.whiteQCastleRight && get(0, 3) == EMPTY && get(0, 2) == EMPTY && get(0, 1) == EMPTY) {
            addCastleMove(&moves, WHITE_QUEEN_SIDE_CASTLE);
        } 
    } else {
        if(board.blackKCastleRight && get(7, 5) == EMPTY && get(7, 6) == EMPTY) {
            addCastleMove(&moves, BLACK_KING_SIDE_CASTLE);
        }
        if(board.blackQCastleRight && get(7, 3) == EMPTY && get(7, 2) == EMPTY && get(7, 1) == EMPTY) {
            addCastleMove(&moves, BLACK_QUEEN_SIDE_CASTLE);
        }
    }

    int perspective = (colorToMove == WHITE) ? 1 : -1;
    //pawn moves
    PosNode *current = (colorToMove == WHITE) ? board.whitePawns.head : board.blackPawns.head;
    
    while(current != NULL) {
        if(board.board[current->pos + perspective*8] == EMPTY) {
            if(current->pos/8 == (3.5-perspective*2.5) && board.board[current->pos + perspective*16] == EMPTY) {
                addPawnMove(&moves, current->pos, current->pos + perspective*16, colorToMove);
            }
            addPawnMove(&moves, current->pos, current->pos + perspective*8, colorToMove);
        }
        if(current->pos%8 != (3.5-perspective*3.5) && canTake(board.board[current->pos + perspective*7], colorToMove)) {
            addPawnMove(&moves, current->pos, current->pos + perspective*7, colorToMove);
        }

        if(current->pos%8 != (3.5+perspective*3.5) && canTake(board.board[current->pos + perspective*9], colorToMove)) {
            addPawnMove(&moves, current->pos, current->pos + perspective*9, colorToMove);
        }
        if(current->pos%8 != (3.5+perspective*3.5) && current->pos + perspective*1 == board.enPassantTarget) {
            addEnPassantMove(&moves, current->pos, current->pos + perspective*9);
        } else if(current->pos%8 != (3.5-perspective*3.5) && current->pos - perspective*1 == board.enPassantTarget) {
            addEnPassantMove(&moves, current->pos, current->pos + perspective*7);
        }
        current = current->next;
    }

    //knight moves
    current = (colorToMove == WHITE) ? board.whiteKnights.head : board.blackKnights.head;
    while(current != NULL) {
        int file = current->pos%8;
        int rank = current->pos/8;
        addMove2(&moves, current->pos, rank+1, file+2, colorToMove);      
        addMove2(&moves, current->pos, rank+2, file+1, colorToMove);
        addMove2(&moves, current->pos, rank-1, file+2, colorToMove);
        addMove2(&moves, current->pos, rank-2, file+1, colorToMove);
        addMove2(&moves, current->pos, rank+1, file-2, colorToMove);
        addMove2(&moves, current->pos, rank+2, file-1, colorToMove);
        addMove2(&moves, current->pos, rank-1, file-2, colorToMove);
        addMove2(&moves, current->pos, rank-2, file-1, colorToMove);
        current = current->next;
    }

    current = (colorToMove == WHITE) ? board.whiteBishops.head : board.blackBishops.head;
    while(current != NULL) {
        addDiagonalMoves(&moves, current->pos, TRUE);
        current = current->next;
    }

    current = (colorToMove == WHITE) ? board.whiteRooks.head : board.blackRooks.head;
    while(current != NULL) {
        addSlidingMoves(&moves, current->pos, TRUE);
        current = current->next;
    }
    
    current = (colorToMove == WHITE) ? board.whiteQueens.head : board.blackQueens.head;
    while(current != NULL) {
        addDiagonalMoves(&moves, current->pos, TRUE);
        addSlidingMoves(&moves, current->pos, TRUE);
        current = current->next;
    }
    
    int kingPos = (colorToMove == WHITE) ? board.whiteKing : board.blackKing;
    int kingRank = kingPos/8;
    int kingFile = kingPos%8;
    
    addMove2(&moves, kingPos, kingRank + 1, kingFile + 1, colorToMove);
    addMove2(&moves, kingPos, kingRank + 1, kingFile + 0, colorToMove);
    addMove2(&moves, kingPos, kingRank + 1, kingFile - 1, colorToMove);
    addMove2(&moves, kingPos, kingRank + 0, kingFile + 1, colorToMove);
    addMove2(&moves, kingPos, kingRank + 0, kingFile - 1, colorToMove);
    addMove2(&moves, kingPos, kingRank - 1, kingFile + 1, colorToMove);
    addMove2(&moves, kingPos, kingRank - 1, kingFile + 0, colorToMove);
    addMove2(&moves, kingPos, kingRank - 1, kingFile - 1, colorToMove);

    return moves;
}

int inCheck(int player, Moves opponentMoves) {

    int kingPos;
    if(player == WHITE) {
        kingPos = board.whiteKing;
    } else {
        kingPos = board.blackKing;
    }

    MoveNode *current = opponentMoves.head;
    while(current != NULL) {
        if(kingPos == current->dest) {
            return TRUE;
        }
        current = current->next;
    }

    return FALSE;
}

PinNode *getPin(Pins pins, int pos) {
    PinNode *current = pins.head;
    while(current != NULL) {
        if(current->pieceIndex == pos) return current;
        current = current->next;
    }
    return NULL;
}

int fullyLegalMove(MoveNode move) {
   PinNode *pin;
   if(!containsPosition(board.attackedSquares, (board.turn == WHITE) ? board.whiteKing : board.blackKing)) {
        if(move.castle != NO_CASTLE) {
            int safe[2];
            if(move.castle == WHITE_KING_SIDE_CASTLE) {
                safe[0] = 5;
                safe[1] = 6;
            } else if(move.castle == WHITE_QUEEN_SIDE_CASTLE) {
                safe[0] = 3;
                safe[1] = 2;
            } else if(move.castle == BLACK_KING_SIDE_CASTLE) {
                safe[0] = 62;
                safe[1] = 61;
            } else if(move.castle == BLACK_QUEEN_SIDE_CASTLE) {
                safe[0] = 59;
                safe[1] = 58;
            }
            PosNode *current = board.attackedSquares.head;
            while(current != NULL) {
                if(current->pos == safe[0] || current->pos == safe[1]) return FALSE;
                current = current->next;
            }
            return TRUE;
        } else if(piece(board.board[move.origin]) == KING) {
            return !containsPosition(board.attackedSquares, move.dest);
        } else if((pin = getPin(board.pins, move.origin)) != NULL) {
            //char sq[3];
            //getSquareChar(move.origin, sq);
            //printf("%s: ", sq); printPositions(*(pin->pinLine));
            if(pin->enPassantOnly) {
                return move.enPassantTarget == -1;
            }
            return containsPosition(*(pin->pinLine), move.dest);
        }
   } else {
        if(piece(board.board[move.origin]) == KING && !containsPosition(board.attackedSquares, move.dest)) {
            return TRUE;
        } else if(piece(board.board[move.origin]) != KING && board.checkers.len == 1) {
            if((pin = getPin(board.pins, move.origin)) != NULL && !containsPosition(*(pin->pinLine), move.dest)) {
                if(pin->enPassantOnly) {
                    return move.enPassantTarget == -1;
                }
                return FALSE;
            }
            if(board.checkers.head->pinLine != NULL) {
                if(containsPosition(*(board.checkers.head->pinLine), move.dest)) return TRUE;
            } else if(board.checkers.head->pieceIndex == move.dest || board.checkers.head->pieceIndex == move.enPassantTarget) {
                return TRUE;
            }
        }
        return FALSE;
   }
   return TRUE;
}

/*
int fullyLegalMove(MoveNode move) {

    
    int legal = TRUE;
    if(move.castle == NO_CASTLE) {
        int lastEnPassantTarget = board.enPassantTarget;
        int lastWhiteKCastleRight = board.whiteKCastleRight;
        int lastWhiteQCastleRight = board.whiteQCastleRight;
        int lastBlackKCastleRight = board.blackKCastleRight;
        int lastBlackQCastleRight = board.blackQCastleRight;
        int lastCapture = makeMove(move); 
        Moves opponentMoves = getPseudoLegalMoves(board.turn);
        legal = 1 - inCheck(inv(board.turn), opponentMoves);
        unMakeMove(move, lastCapture, lastEnPassantTarget);
        board.whiteKCastleRight = lastWhiteKCastleRight;
        board.whiteQCastleRight = lastWhiteQCastleRight;
        board.blackKCastleRight = lastBlackKCastleRight;
        board.blackQCastleRight = lastBlackQCastleRight;
    } else {
        int safe[3];
        int pawnFree;
        Moves opponentMoves = getPseudoLegalMoves(inv(board.turn));
        if(move.castle == WHITE_KING_SIDE_CASTLE) {
            safe[0] = 4;
            safe[1] = 5;
            safe[2] = 6;
            pawnFree = 11;
        } else if(move.castle == WHITE_QUEEN_SIDE_CASTLE) {
            safe[0] = 4;
            safe[1] = 3;
            safe[2] = 2;
            pawnFree = 9;
        } else if(move.castle == BLACK_KING_SIDE_CASTLE) {
            safe[0] = 62;
            safe[1] = 61;
            safe[2] = 60;
            pawnFree = 51;
        } else if(move.castle == BLACK_QUEEN_SIDE_CASTLE) {
            safe[0] = 60;
            safe[1] = 59;
            safe[2] = 58;
            pawnFree = 49;
        }

        MoveNode *current = opponentMoves.head;
        while(current != NULL) {
            
            for(int i = pawnFree; i < pawnFree + 5; i ++) {
                if(board.board[i] == (PAWN | inv(board.turn))) return FALSE;
            }
            if(safe[0] == current->dest || safe[1] == current->dest || safe[2] == current->dest) return FALSE;
            current = current->next;
        }
    }

    return legal;
}
*/

Moves getLegalMoves() {
    Moves pseudoLegalMoves = getPseudoLegalMoves(board.turn);
    Moves fullyLegalMoves = {
        .len = 0,
        .head = NULL,
        .tail = NULL
    };

    clearPositions(&board.attackedSquares);
    clearPins(&board.pins);
    clearPins(&board.checkers);
    
    getAttackedSquaresPinsAndCheckers(&board.attackedSquares, &board.pins, &board.checkers);
    MoveNode *current = pseudoLegalMoves.head;
    while(current != NULL) {
        if(fullyLegalMove(*current)) {
            MoveNode *copy;
            copy = (MoveNode *)fred(1, sizeof(MoveNode));

            copy->castle = current->castle;
            copy->dest = current->dest;
            copy->enPassantTarget = current->enPassantTarget;
            copy->promotion = current->promotion;
            copy->next = NULL;
            copy->origin = current->origin;
            addMoveNode(&fullyLegalMoves, copy);
        }
        current = current->next;
    }
    
    
    clearMoves(&pseudoLegalMoves);
    return fullyLegalMoves;
}

MoveNode parseMove(char **args) {
    MoveNode move;

    move.origin = -1;
    move.dest = -1;
    move.enPassantTarget = -1;
    move.castle = NO_CASTLE;
    move.promotion = NO_PROMOTION;

    if(strcmp(args[1], "O-O-O") == 0) {
        move.castle = (board.turn == WHITE) ? WHITE_QUEEN_SIDE_CASTLE : BLACK_QUEEN_SIDE_CASTLE;
    } else if(strcmp(args[1], "O-O") == 0) {
        move.castle = (board.turn == WHITE) ? WHITE_KING_SIDE_CASTLE : BLACK_KING_SIDE_CASTLE;
    } else if(strcmp(args[1], "promote") == 0) {
        move.origin = parseSquare(args[2]);
        move.dest = parseSquare(args[3]);
        move.promotion = args[4][0];
    } else if(strcmp(args[1], "ep")  == 0) {
        move.enPassantTarget = board.enPassantTarget;
        move.origin = parseSquare(args[2]);
        move.dest = parseSquare(args[3]);
    } else {
        move.origin = parseSquare(args[1]);
        move.dest = parseSquare(args[2]);       
    }

    return move;
}

void makeRandomMove() {
    Moves moves = getLegalMoves();
    if(moves.len != 0) {
        int index = (int)((rand() / (double)RAND_MAX) * moves.len);
        printf("Index: %d", index);
        MoveNode *curr = moves.head;
        for(int i = 0; i < index; i ++) {
            curr = curr->next;
        }
        makeMove(*curr);
    }
    clearMoves(&moves);
}

int countMoves(int depth) {
    Moves moves = getLegalMoves();
    int numMoves = 0;
    if(depth == 1) {
        int len = moves.len;
        clearMoves(&moves);
        return len;
    } else {
        MoveNode *current = moves.head;
        while(current != NULL) {
            int lastWhiteKCastleRight = board.whiteKCastleRight;
            int lastWhiteQCaslteRight = board.whiteQCastleRight;
            int lastBlackKCastleRight = board.blackKCastleRight;
            int lastBlackQCastleRight = board.blackQCastleRight;
            int lastEnPassantTarget = board.enPassantTarget;
            
            int timeWasted = 0;
            int time = micros();
            int capture = makeMove(*current);
            numMoves += countMoves(depth-1);
            unMakeMove(*current, capture, lastEnPassantTarget);
            timeWasted += micros() - time;
            board.whiteKCastleRight = lastWhiteKCastleRight;
            board.whiteQCastleRight = lastWhiteQCaslteRight;
            board.blackKCastleRight = lastBlackKCastleRight;
            board.blackQCastleRight = lastBlackQCastleRight;

            current = current->next;
        }
    }

    clearMoves(&moves);
    return numMoves;
}

/*
36: 1
39: 3
41: 4
42: 11
43: 20
44: 6
45: 2
46: 1
*/

/*
Nodes: 43
Nodes: 43
Nodes: 41
Nodes: 46
Nodes: 44
Nodes: 44
Nodes: 42
Nodes: 42
Nodes: 42
Nodes: 43
Nodes: 42
Nodes: 45
Nodes: 44
Nodes: 42
Nodes: 45
Nodes: 42
Nodes: 43
Nodes: 42
Nodes: 42
Nodes: 39
Nodes: 42
Nodes: 43
Nodes: 43
Nodes: 43
Nodes: 42
Nodes: 41
Nodes: 44
Nodes: 44
Nodes: 42
Nodes: 41
Nodes: 39
Nodes: 37
Nodes: 43
Nodes: 43
Nodes: 43
Nodes: 43
Nodes: 43
Nodes: 43
Nodes: 43
Nodes: 43
Nodes: 43
Nodes: 43
Nodes: 45
Nodes: 43
Nodes: 42
Nodes: 39
Nodes: 43
Nodes: 43
*/

int execute(char *command) {

    int argc;
    char **args;
    args = init();

    parse(command, args, &argc);

    if(strcmp(args[0], "load") == 0) {
        loadFENStr(args[1]);
        printBoard();
    } else if(strcmp(args[0], "move") == 0) {

        prevBlackKCastleRight = board.blackKCastleRight;
        prevBlackQCastleRight = board.blackQCastleRight;
        prevWhiteKCastleRight = board.whiteKCastleRight;
        prevWhiteQCastleRight = board.whiteQCastleRight;

        Moves moves = getLegalMoves();
        MoveNode move = parseMove(args);
        if(containsMove(moves, move)) {
            lastMove = move;
            prevEnPassantTarget = board.enPassantTarget;
            prevCapturedPiece = makeMove(move);
            printBoard();
        } else {
            printf("Illegal Move\n");
        }
        clearMoves(&moves);
        
    } else if(strcmp(args[0], "quit") == 0) {
        return TRUE;
    } else if(strcmp(args[0], "info") == 0) {
        printf((board.turn == WHITE) ? "\nWhite to move\n" : "\nBlack to move\n");
        printf("%d\n", board.turn);
        printf("BlackKCastleRight: %d \n", board.blackKCastleRight);
        printf("BlackQCastleRight: %d \n", board.blackQCastleRight);
        printf("WhiteKCastleRight: %d \n", board.whiteKCastleRight);
        printf("WhiteQCastleRight: %d \n", board.whiteQCastleRight);
        printf("En Passant Target: %d \n", board.enPassantTarget);
        printf("Half Moves Completed: %d \n", board.halfMoves);
        printf("Full Move Clock: %d \n\n", board.fullMoves);
    } else if(strcmp(args[0], "board") == 0) {
        printBoard();
    } else if(strcmp(args[0], "moves") == 0) {
        unsigned long start = micros();
        Moves moves = getLegalMoves(board.turn);
        printf("Time: %ld\n", micros() - start);
        
        printMoves(moves);
        clearMoves(&moves);
    } else if(strcmp(args[0], "positions") == 0) {

        printf("White \n");
        printf("Pawns: "); printPositions(board.whitePawns);
        printf("Knights: "); printPositions(board.whiteKnights);
        printf("Bishops: "); printPositions(board.whiteBishops);
        printf("Rooks: "); printPositions(board.whiteRooks);
        printf("Queens: "); printPositions(board.whiteQueens);
        
        printf("Black \n");
        printf("Pawns: "); printPositions(board.blackPawns);
        printf("Knights: "); printPositions(board.blackKnights);
        printf("Bishops: "); printPositions(board.blackBishops);
        printf("Rooks: "); printPositions(board.blackRooks);
        printf("Queens: "); printPositions(board.blackQueens);
    } else if(strcmp(args[0], "undo") == 0) {
        unMakeMove(lastMove, prevCapturedPiece, prevEnPassantTarget);
        board.whiteKCastleRight = prevWhiteKCastleRight;
        board.whiteQCastleRight = prevWhiteQCastleRight;
        board.blackKCastleRight = prevBlackKCastleRight;
        board.blackQCastleRight = prevBlackQCastleRight;
        printBoard();
    } else if(strcmp(args[0], "check") == 0) {
        printf("In check: %d", inCheck(board.turn, getPseudoLegalMoves(inv(board.turn))));
    } else if(strcmp(args[0], "rand") == 0) {
        int moves = atoi(args[1]);
        printf("Moves: %d", moves);
        for(int i = 0; i < moves; i ++) {
            makeRandomMove();  
            printBoard();  
        }
        
    } else if(strcmp(args[0], "stress") == 0) {
        loadFENStr(STARTING_POS);
        Moves moves = getLegalMoves();
        int numMoves = atoi(args[1]);
        MoveNode *curr = moves.head;
        int randCalls = 0;
        while(curr != NULL) {    
            makeMove(*curr);
            printBoard();
            printf("Rand calls: %d", randCalls);
            printf("White \n");
            printf("Pawns: "); printPositions(board.whitePawns);
            printf("Knights: "); printPositions(board.whiteKnights);
            printf("Bishops: "); printPositions(board.whiteBishops);
            printf("Rooks: "); printPositions(board.whiteRooks);
            printf("Queens: "); printPositions(board.whiteQueens);
            
            printf("Black \n");
            printf("Pawns: "); printPositions(board.blackPawns);
            printf("Knights: "); printPositions(board.blackKnights);
            printf("Bishops: "); printPositions(board.blackBishops);
            printf("Rooks: "); printPositions(board.blackRooks);
            printf("Queens: "); printPositions(board.blackQueens);
            for(int i = 0; i < numMoves; i ++) {
                makeRandomMove();
                randCalls ++;
            }
            //printBoard();
            loadFENStr(STARTING_POS);
            curr = curr->next;
        }
    } else if(strcmp(args[0], "perft") == 0) {
        Moves moves = getLegalMoves();
        MoveNode *current = moves.head;
        int totalNodes = 0;
        while(current != NULL) {
            int enPassantTarget = board.enPassantTarget;
            int whiteKCastleRight = board.whiteKCastleRight;
            int whiteQCastleRight = board.whiteQCastleRight;
            int blackKCastleRight = board.blackKCastleRight;
            int blackQCastleRight = board.blackQCastleRight;
            unsigned char capture = makeMove(*current);
            int nodes = countMoves(atoi(args[1])-1);
            char origin[3];
            char dest[3];
            getSquareChar(current->origin, origin);
            getSquareChar(current->dest, dest);
            printf("%s%s: %d\n", origin, dest, nodes);
            totalNodes += nodes;
            unMakeMove(*current, capture, enPassantTarget);
            board.whiteKCastleRight = whiteKCastleRight;
            board.whiteQCastleRight = whiteQCastleRight;
            board.blackKCastleRight = blackKCastleRight;
            board.blackQCastleRight = blackQCastleRight;
            current = current->next;
        }
        clearMoves(&moves);
        printf("Nodes: %d\n", totalNodes);
        moveCount = 0;
    } else if(strcmp(args[0], "break") == 0) {
        for(int i = 0; i < 1000; i ++) {
            makeRandomMove();
        }
        loadFENStr("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8");
        for(int i = 0; i < atoi(args[1]); i ++) {
            makeRandomMove();
            printBoard();
        }
    } else if(strcmp(args[0], "opponentMoves") == 0) {
        struct timeval tv;
        gettimeofday(&tv,NULL);
        unsigned long time_in_micros_1 = 1000000 * tv.tv_sec + tv.tv_usec;
        Moves moves = getPseudoLegalMoves(inv(board.turn));
        gettimeofday(&tv,NULL);
        unsigned long time_in_micros_2 = 1000000 * tv.tv_sec + tv.tv_usec;
        printf("Time: %ld\n", time_in_micros_2 - time_in_micros_1);
        
        printMoves(moves);
    } else if(strcmp(args[0], "movetest") == 0) {
        unsigned long micros_t = micros();
        Moves moves = getLegalMoves(board.turn);
        if(moves.len != 0) {
            int index = (int)((rand() / (double)RAND_MAX) * moves.len); 
            MoveNode *move = moves.head;
            for(int i = 0; i < index; i ++) {
                move = move->next;
            }
            int enPassantTarget = board.enPassantTarget;
            int whiteKCastleRight = board.whiteKCastleRight;
            int whiteQCastleRight = board.whiteQCastleRight;
            int blackKCastleRight = board.blackKCastleRight;
            int blackQCastleRight = board.blackQCastleRight;
            unsigned char capture = makeMove(*move);
            unMakeMove(*move, capture, enPassantTarget);
            board.whiteKCastleRight = whiteKCastleRight;
            board.whiteQCastleRight = whiteQCastleRight;
            board.blackKCastleRight = blackKCastleRight;
            board.blackQCastleRight = blackQCastleRight;
        }
    } else if(strcmp(args[0], "test") == 0) {
        Pins pins = {
            .head = NULL,
            .tail = NULL,
            .len = 0
        };
        Positions attackedSquares = {
            .head = NULL,
            .tail = NULL,
            .len = 0
        };
        Pins checkers = {
            .head = NULL,
            .tail = NULL,
            .len = 0
        };
        unsigned long start = micros();
        getAttackedSquaresPinsAndCheckers(&attackedSquares, &pins, &checkers);
        printf("Time: %ld\n", micros() - start);
        printf("Attacked squares: "); printPositions(attackedSquares);
        printf("Checkers\n");
        PinNode *current = checkers.head;
        while(current != NULL) {
            char position[3];
            getSquareChar(current->pieceIndex, position);
            printf("%s: ", position); if(current->pinLine != NULL) printPositions(*(current->pinLine));
            current = current->next;
        }
        printf("\nPins\n");
        current = pins.head;
        while(current != NULL) {
            char position[3];
            getSquareChar(current->pieceIndex, position);
            printf("%s, e.p only (%d): ", position, current->enPassantOnly); printPositions(*(current->pinLine));
            current = current->next;
        }
    } else if(strcmp(args[0], "mem") == 0) {
        printf("Mem: %d", memCount);
    }

    keel(*args);
    keel(args);
    return FALSE;
}

int main() {

    printf("-----------------------------------------------------------------------------------------------------------------------------\n\n");
    printf("        CCCCCCCCCCCCC                                    lllllll    SSSSSSSSSSSSSSS IIIIIIIIIIMMMMMMMM               MMMMMMMM\n");
    printf("     CCC::::::::::::C                                    l:::::l  SS:::::::::::::::SI::::::::IM:::::::M             M:::::::M\n");
    printf("   CC:::::::::::::::C                                    l:::::l S:::::SSSSSS::::::SI::::::::IM::::::::M           M::::::::M\n");
    printf("  C:::::CCCCCCCC::::C                                    l:::::l S:::::S     SSSSSSSII::::::IIM:::::::::M         M:::::::::M\n");
    printf(" C:::::C       CCCCCC  aaaaaaaaaaaaa  rrrrr   rrrrrrrrr   l::::l S:::::S              I::::I  M::::::::::M       M::::::::::M\n");
    printf("C:::::C                a::::::::::::a r::::rrr:::::::::r  l::::l S:::::S              I::::I  M:::::::::::M     M:::::::::::M\n");
    printf("C:::::C                aaaaaaaaa:::::ar:::::::::::::::::r l::::l  S::::SSSS           I::::I  M:::::::M::::M   M::::M:::::::M\n");
    printf("C:::::C                         a::::arr::::::rrrrr::::::rl::::l   SS::::::SSSSS      I::::I  M::::::M M::::M M::::M M::::::M\n");
    printf("C:::::C                  aaaaaaa:::::a r:::::r     r:::::rl::::l     SSS::::::::SS    I::::I  M::::::M  M::::M::::M  M::::::M\n");
    printf("C:::::C                aa::::::::::::a r:::::r     rrrrrrrl::::l        SSSSSS::::S   I::::I  M::::::M   M:::::::M   M::::::M\n");
    printf("C:::::C               a::::aaaa::::::a r:::::r            l::::l             S:::::S  I::::I  M::::::M    M:::::M    M::::::M\n");
    printf("C:::::C       CCCCCCa::::a    a:::::a r:::::r            l::::l             S:::::S  I::::I  M::::::M     MMMMM     M::::::M\n");
    printf("  C:::::CCCCCCCC::::Ca::::a    a:::::a r:::::r           l::::::lSSSSSSS     S:::::SII::::::IIM::::::M               M::::::M\n");
    printf("   CC:::::::::::::::Ca:::::aaaa::::::a r:::::r           l::::::lS::::::SSSSSS:::::SI::::::::IM::::::M               M::::::M\n");
    printf("     CCC::::::::::::C a::::::::::aa:::ar:::::r           l::::::lS:::::::::::::::SS I::::::::IM::::::M               M::::::M\n");
    printf("        CCCCCCCCCCCCC  aaaaaaaaaa  aaaarrrrrrr           llllllll SSSSSSSSSSSSSSS   IIIIIIIIIIMMMMMMMM               MMMMMMMM\n");
    printf("\n-----------------------------------------------------------------------------------------------------------------------------\n\n\n");

    loadFENStr(STARTING_POS);
    printBoard();
    
    char *command = NULL;
    size_t len = 0;

    while(1) {

        while(len == 0) {
            getline(&command, &len, stdin);
        }
        len = 0;

        int quit = execute(command);
        if(quit) return 0;      
    }
    return 0;
}
