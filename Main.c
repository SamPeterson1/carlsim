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
#define WHITE 'w'
#define BLACK 'b'
#define get(rank, file) board.board[rank*8+file]
#define set(rank, file, piece)board.board[rank*8+file] = piece

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
    char promotion;

} MoveNode;

typedef struct Moves_s {

    int len;
    MoveNode *head;
    MoveNode *tail;

} Moves;

Moves blah;

typedef struct Board_s {
    char board[64];
    char turn;

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
} Board;

Board board;

MoveNode lastMove;
int prevEnPassantTarget;
char prevCapturedPiece;

int prevBlackKCastleRight;
int prevBlackQCastleRight;
int prevWhiteKCastleRight;
int prevWhiteQCastleRight;

int moveCount;

void printBoard() {    
    printf("\e[1;1H\e[2J");
    for(int rank = 7; rank >= 0; rank --) {
        printf("    +---+---+---+---+---+---+---+---+\n");
        printf("(%d) ", rank+1);
        for(int file = 0; file < 8; file ++) {
            printf("| %c ", get(rank, file));
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
    node = (MoveNode *)calloc(1, sizeof(MoveNode));

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

void addPromotionMove(Moves *moves, int origin, int dest, char promotion) {
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

    node = (PosNode *)calloc(1, sizeof(PosNode));
    
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

void clearPositions(Positions *positions) {
    PosNode *current = positions->head;
    while(current != NULL) {   
        PosNode *next = current->next;
        free(current);
        current = next;
    }
    positions->len = 0;
    positions->head = NULL;
    positions->tail = NULL;
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
            free(current);
            break;
        }
        last = current;
        current = current->next;
    }
}

char **init()
{

    char **ret;

    ret = (char **)calloc(1, MAX_ARG_COUNT * sizeof(char *));
    *(ret + 0) = (char *)calloc(1, MAX_ARG_COUNT * (MAX_ARG_LENGTH + 1) * sizeof(char));
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
            set(rank, file, ' ');
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
            set(rank, file, FENCode[i]);
            switch(FENCode[i]) {
                case 'p':
                    addPosition(&board.blackPawns, rank*8 + file);
                    break;
                case 'n':
                    addPosition(&board.blackKnights, rank*8 + file);
                    break;
                case 'b':
                    addPosition(&board.blackBishops, rank*8 + file);
                    break;
                case 'r':
                    addPosition(&board.blackRooks, rank*8 + file);
                    break;
                case 'q':
                    addPosition(&board.blackQueens, rank*8 + file);
                    break;
                case 'k':
                    board.blackKing = rank*8 + file;
                    break;
                case 'P':
                    addPosition(&board.whitePawns, rank*8 + file);
                    break;
                case 'N':
                    addPosition(&board.whiteKnights, rank*8 + file);
                    break;
                case 'B':
                    addPosition(&board.whiteBishops, rank*8 + file);
                    break;
                case 'R':
                    addPosition(&board.whiteRooks, rank*8 + file);
                    break;
                case 'Q':
                    addPosition(&board.whiteQueens, rank*8 + file);
                    break;
                case 'K':
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
    board.turn = FENCode[i];
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

Positions *getPositionsOfType(char piece) {
    switch(piece) {
        case 'p':
            return &board.blackPawns;
        case 'n':
            return &board.blackKnights;
        case 'b':
            return &board.blackBishops;
        case 'r':
            return &board.blackRooks;
        case 'q':
            return &board.blackQueens;
        case 'P':
            return &board.whitePawns;
        case 'N':
            return &board.whiteKnights;
        case 'B':
            return &board.whiteBishops;
        case 'R':
            return &board.whiteRooks;
        case 'Q':
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

void unMakeMove(MoveNode move, char capturedPiece, int lastEnPassantTarget) {
    
    if(move.enPassantTarget != -1) {
        board.board[move.enPassantTarget] = capturedPiece;
        Positions *capturedPosition = getPositionsOfType(capturedPiece);
        addPosition(capturedPosition, move.enPassantTarget);

        Positions *movedPosition = getPositionsOfType(board.board[move.dest]);
        removePosition(movedPosition, move.dest);
        addPosition(movedPosition, move.origin);
        board.board[move.origin] = board.board[move.dest];
        board.board[move.dest] = ' ';
    } else if(move.castle != NO_CASTLE) {
        if(move.castle == WHITE_KING_SIDE_CASTLE) {
            board.board[7] = 'R';
            board.board[4] = 'K';
            board.board[6] = ' ';
            board.board[5] = ' ';
            board.whiteKing = 4;
            addPosition(&board.whiteRooks, 7);
            removePosition(&board.whiteRooks, 5);
        } else if(move.castle == WHITE_QUEEN_SIDE_CASTLE) {
            board.board[0] = 'R';
            board.board[4] = 'K';
            board.board[2] = ' ';
            board.board[3] = ' ';
            board.whiteKing = 4;
            addPosition(&board.whiteRooks, 0);
            removePosition(&board.whiteRooks, 3);
        } else if(move.castle == BLACK_KING_SIDE_CASTLE) {
            board.board[63] = 'r';
            board.board[60] = 'k';
            board.board[62] = ' ';
            board.board[61] = ' ';
            board.blackKing = 60;
            addPosition(&board.blackRooks, 63);
            removePosition(&board.blackRooks, 61);
        } else if(move.castle == BLACK_QUEEN_SIDE_CASTLE) {
            board.board[56] = 'r';
            board.board[60] = 'k';
            board.board[58] = ' ';
            board.board[59] = ' ';
            board.blackKing = 60;
            addPosition(&board.blackRooks, 56);
            removePosition(&board.blackRooks, 59);
        }
    } else {
        if(capturedPiece == 'K') {
            board.whiteKing = move.dest;
            board.board[move.dest] = 'K';
        } else if(capturedPiece == 'k') {
            board.board[move.dest] = 'k';
            board.blackKing = move.dest;
        } else if(capturedPiece != ' ') {
            Positions *capturedPosition = getPositionsOfType(capturedPiece);
            addPosition(capturedPosition, move.dest);
        }
        if(board.board[move.dest] == 'K') {
           board.whiteKing = move.origin;
           board.board[move.origin] = 'K';
           board.board[move.dest] = capturedPiece;
       } else if(board.board[move.dest] == 'k') {
           board.blackKing = move.origin;
           board.board[move.origin] = 'k';
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
        board.board[move.origin] = (board.turn == WHITE) ? 'p' : 'P';
    }

    board.enPassantTarget = lastEnPassantTarget;
    
    if(board.turn == WHITE) board.turn = BLACK;
    else board.turn = WHITE;
}

char makeMove(MoveNode move) {
    char capture = ' ';
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
       board.board[move.origin] = ' ';
       capture = board.board[move.enPassantTarget];
       board.board[move.enPassantTarget] = ' ';
    } else if(move.castle != NO_CASTLE) {
        if(move.castle == WHITE_KING_SIDE_CASTLE) {
            board.board[7] = ' ';
            board.board[4] = ' ';
            board.board[6] = 'K';
            board.board[5] = 'R';
            board.whiteKing = 6;
            board.whiteKCastleRight = FALSE;
            board.whiteQCastleRight = FALSE;
            removePosition(&board.whiteRooks, 7);
            addPosition(&board.whiteRooks, 5);
        } else if(move.castle == WHITE_QUEEN_SIDE_CASTLE) {
            board.board[0] = ' ';
            board.board[4] = ' ';
            board.board[2] = 'K';
            board.board[3] = 'R';
            board.whiteKing = 2;
            board.whiteKCastleRight = FALSE;
            board.whiteQCastleRight = FALSE;
            removePosition(&board.whiteRooks, 0);
            addPosition(&board.whiteRooks, 3);
        } else if(move.castle == BLACK_KING_SIDE_CASTLE) {
            board.board[63] = ' ';
            board.board[60] = ' ';
            board.board[62] = 'k';
            board.board[61] = 'r';
            board.blackKing = 62;
            board.blackKCastleRight = FALSE;
            board.blackQCastleRight = FALSE;
            removePosition(&board.blackRooks, 63);
            addPosition(&board.blackRooks, 61);
        } else if(move.castle == BLACK_QUEEN_SIDE_CASTLE) {
            board.board[56] = ' ';
            board.board[60] = ' ';
            board.board[58] = 'K';
            board.board[59] = 'R';
            board.blackKing = 58;
            board.blackKCastleRight = FALSE;
            board.blackQCastleRight = FALSE;
            removePosition(&board.blackRooks, 56);
            addPosition(&board.blackRooks, 59);
        }
    } else {
        if(board.board[move.dest] != ' ' && board.board[move.dest] != 'K' && board.board[move.dest] != 'k') {
            Positions *capturedPosition = getPositionsOfType(board.board[move.dest]);
            removePosition(capturedPosition, move.dest);
            capture = board.board[move.dest];
        } else {
            capture = board.board[move.dest];
        }
        if(board.board[move.origin] == 'K') {
            board.whiteKCastleRight = FALSE;
            board.whiteQCastleRight = FALSE;
            board.whiteKing = move.dest;
            capture = board.board[move.dest];
            board.board[move.dest] = 'K';
            board.board[move.origin] = ' ';  
        } else if(board.board[move.origin] == 'k') {
            board.blackKCastleRight = FALSE;
            board.blackQCastleRight = FALSE;
            board.blackKing = move.dest;
            capture = board.board[move.dest];
            board.board[move.dest] = 'k';
            board.board[move.origin] = ' ';
        } else {
            Positions *movedPosition = getPositionsOfType(board.board[move.origin]);
            
            removePosition(movedPosition, move.origin);
            addPosition(movedPosition, move.dest);

            board.board[move.dest] = board.board[move.origin];
            board.board[move.origin] = ' ';

            if((board.board[move.dest] == 'p' || board.board[move.dest] == 'P') && abs(move.dest - move.origin) == 16) {
                board.enPassantTarget = move.dest;
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

    if(board.turn == WHITE) board.turn = BLACK;
    else board.turn = WHITE;

   return capture;
}

int split (const char *txt, char delim, char ***tokens)
{
    int *tklen, *t, count = 1;
    char **arr, *p = (char *) txt;

    while (*p != '\0') if (*p++ == delim) count += 1;
    t = tklen = calloc (count, sizeof (int));
    for (p = (char *) txt; *p != '\0'; p++) *p == delim ? *t++ : (*t)++;
    *tokens = arr = malloc (count * sizeof (char *));
    t = tklen;
    p = *arr++ = calloc (*(t++) + 1, sizeof (char *));
    while (*txt != '\0')
    {
        if (*txt == delim)
        {
            p = *arr++ = calloc (*(t++) + 1, sizeof (char *));
            txt++;
        }
        else *p++ = *txt++;
    }
    free (tklen);
    return count;
}

int canTake(char piece, int colorToMove) {
    if(piece == ' ') return FALSE;
    if(piece > 96) return colorToMove == WHITE;
    return colorToMove == BLACK;
}

int canMoveTo(char piece, int colorToMove) {
    if(piece == ' ') return TRUE;
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
    if(get(destRank, destFile) != ' ') {
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

void addPawnMove(Moves *moves, int origin, int dest, int colorToMove) {
    if(colorToMove == WHITE && dest/8 == 7) {
        addPromotionMove(moves, origin, dest, 'Q');
        addPromotionMove(moves, origin, dest, 'R');
        addPromotionMove(moves, origin, dest, 'N');
        addPromotionMove(moves, origin, dest, 'B');
    } else if(colorToMove == BLACK && dest/8 == 0) {
        addPromotionMove(moves, origin, dest, 'q');
        addPromotionMove(moves, origin, dest, 'r');
        addPromotionMove(moves, origin, dest, 'n');
        addPromotionMove(moves, origin, dest, 'b');
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
        if(board.whiteKCastleRight && get(0, 5) == ' ' && get(0, 6) == ' ') {
            addCastleMove(&moves, WHITE_KING_SIDE_CASTLE);
        }
        if(board.whiteQCastleRight && get(0, 3) == ' ' && get(0, 2) == ' ' && get(0, 1) == ' ') {
            addCastleMove(&moves, WHITE_QUEEN_SIDE_CASTLE);
        } 
    } else {
        if(board.blackKCastleRight && get(7, 5) == ' ' && get(7, 6) == ' ') {
            addCastleMove(&moves, BLACK_KING_SIDE_CASTLE);
        }
        if(board.blackQCastleRight && get(7, 3) == ' ' && get(7, 2) == ' ' && get(7, 1) == ' ') {
            addCastleMove(&moves, BLACK_QUEEN_SIDE_CASTLE);
        }
    }

    int perspective = (colorToMove == WHITE) ? 1 : -1;
    //pawn moves
    PosNode *current = (colorToMove == WHITE) ? board.whitePawns.head : board.blackPawns.head;
    
    while(current != NULL) {
        if(board.board[current->pos + perspective*8] == ' ') {
            if(current->pos/8 == (3.5-perspective*2.5) && board.board[current->pos + perspective*16] == ' ') {
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

        int file = current->pos%8;
        int rank = current->pos/8;
        int NEBlocked = FALSE;
        int SEBlocked = FALSE;
        int SWBlocked = FALSE;
        int NWBlocked = FALSE;

        for(int offset = 1; offset < 8; offset ++) {
            addMove3(&moves, current->pos, rank + offset, file + offset, &NEBlocked, colorToMove);
            addMove3(&moves, current->pos, rank - offset, file + offset, &SEBlocked, colorToMove);
            addMove3(&moves, current->pos, rank - offset, file - offset, &SWBlocked, colorToMove);
            addMove3(&moves, current->pos, rank + offset, file - offset, &NWBlocked, colorToMove);
        }
        
        current = current->next;
    }

    current = (colorToMove == WHITE) ? board.whiteRooks.head : board.blackRooks.head;
    while(current != NULL) {
        int file = current->pos%8;
        int rank = current->pos/8;
        
        int NBlocked = FALSE;
        int EBlocked = FALSE;
        int SBlocked = FALSE;
        int WBlocked = FALSE;
        
        for(int offset = 1; offset < 8; offset ++) {
            addMove3(&moves, current->pos, rank + offset, file, &NBlocked, colorToMove);
            addMove3(&moves, current->pos, rank, file + offset, &EBlocked, colorToMove);
            addMove3(&moves, current->pos, rank - offset, file, &SBlocked, colorToMove);
            addMove3(&moves, current->pos, rank, file - offset, &WBlocked, colorToMove);
        }

        current = current->next;
    }
    
    current = (colorToMove == WHITE) ? board.whiteQueens.head : board.blackQueens.head;
    while(current != NULL) {
        int file = current->pos%8;
        int rank = current->pos/8;

        int NEBlocked = FALSE;
        int SEBlocked = FALSE;
        int SWBlocked = FALSE;
        int NWBlocked = FALSE;

        int NBlocked = FALSE;
        int EBlocked = FALSE;
        int SBlocked = FALSE;
        int WBlocked = FALSE;
        
        for(int offset = 1; offset < 8; offset ++) {
            addMove3(&moves, current->pos, rank + offset, file + offset, &NEBlocked, colorToMove);
            addMove3(&moves, current->pos, rank - offset, file + offset, &SEBlocked, colorToMove);
            addMove3(&moves, current->pos, rank - offset, file - offset, &SWBlocked, colorToMove);
            addMove3(&moves, current->pos, rank + offset, file - offset, &NWBlocked, colorToMove);
            addMove3(&moves, current->pos, rank + offset, file, &NBlocked, colorToMove);
            addMove3(&moves, current->pos, rank, file + offset, &EBlocked, colorToMove);
            addMove3(&moves, current->pos, rank - offset, file, &SBlocked, colorToMove);
            addMove3(&moves, current->pos, rank, file - offset, &WBlocked, colorToMove);
        }

        current = current->next;
    }

    int kingPos = (colorToMove == WHITE) ? board.whiteKing : board.blackKing;
    int kingRank = kingPos/8;
    int kingFile = kingPos%8;

    addMove2(&moves, kingPos, kingRank + 1, kingFile + 1, colorToMove);
    addMove2(&moves, kingPos, kingRank + 1, kingFile, colorToMove);
    addMove2(&moves, kingPos, kingRank + 1, kingFile - 1, colorToMove);
    addMove2(&moves, kingPos, kingRank + 0, kingFile + 1, colorToMove);
    addMove2(&moves, kingPos, kingRank + 0, kingFile - 1, colorToMove);
    addMove2(&moves, kingPos, kingRank - 1, kingFile + 1, colorToMove);
    addMove2(&moves, kingPos, kingRank - 1, kingFile, colorToMove);
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
        legal = 1 - inCheck((board.turn == WHITE) ? BLACK : WHITE, opponentMoves);
        unMakeMove(move, lastCapture, lastEnPassantTarget);
        board.whiteKCastleRight = lastWhiteKCastleRight;
        board.whiteQCastleRight = lastWhiteQCastleRight;
        board.blackKCastleRight = lastBlackKCastleRight;
        board.blackQCastleRight = lastBlackQCastleRight;
    } else {
        int safe[3];
        Moves opponentMoves = getPseudoLegalMoves((board.turn == WHITE) ? BLACK : WHITE);
        if(move.castle == WHITE_KING_SIDE_CASTLE) {
            safe[0] = 4;
            safe[1] = 5;
            safe[2] = 6;
        } else if(move.castle == WHITE_QUEEN_SIDE_CASTLE) {
            safe[0] = 4;
            safe[1] = 3;
            safe[2] = 2;
        } else if(move.castle == BLACK_KING_SIDE_CASTLE) {
            safe[0] = 62;
            safe[1] = 61;
            safe[2] = 60;
        } else if(move.castle == BLACK_QUEEN_SIDE_CASTLE) {
            safe[0] = 60;
            safe[1] = 59;
            safe[2] = 58;
        }
        MoveNode *current = opponentMoves.head;
        while(current != NULL) {
            
            if(safe[0] == current->dest || safe[1] == current->dest || safe[2] == current->dest) {
                legal = FALSE;
                break;
            }
            current = current->next;
        }
    }

    return legal;
}

Moves getLegalMoves() {

    Moves pseudoLegalMoves = getPseudoLegalMoves(board.turn);
    Moves fullyLegalMoves = {
        .len = 0,
        .head = NULL,
        .tail = NULL
    };


    MoveNode *current = pseudoLegalMoves.head;

    while(current != NULL) {
        if(fullyLegalMove(*current)) {
            MoveNode *copy;
            copy = (MoveNode *)calloc(1, sizeof(MoveNode));

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
}

int countMoves(int depth) {
    Moves moves = getLegalMoves();
    int numMoves = 0;
    if(depth == 0) {
        MoveNode *current = moves.head;
        while(current != NULL) {
            char origin[3];
            char dest[3];
            char ep[3];
            getSquareChar(current->origin, origin);
            getSquareChar(current->dest, dest);
            if(current->enPassantTarget != -1) {
                getSquareChar(current->enPassantTarget, ep);
            } else {
                ep[0] = '-'; ep[1] = '1'; ep[2] = '\0';
            }
            current = current->next;
        }
        return moves.len;
    } else {
        MoveNode *current = moves.head;
        while(current != NULL) {
            int lastWhiteKCastleRight = board.whiteKCastleRight;
            int lastWhiteQCaslteRight = board.whiteQCastleRight;
            int lastBlackKCastleRight = board.blackKCastleRight;
            int lastBlackQCastleRight = board.blackQCastleRight;
            int lastEnPassantTarget = board.enPassantTarget;
            
            int capture = makeMove(*current);
            numMoves += countMoves(depth-1);
            unMakeMove(*current, capture, lastEnPassantTarget);

            board.whiteKCastleRight = lastWhiteKCastleRight;
            board.whiteQCastleRight = lastWhiteQCaslteRight;
            board.blackKCastleRight = lastBlackKCastleRight;
            board.blackQCastleRight = lastBlackQCastleRight;

            current = current->next;
        }
    }

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
        
    } else if(strcmp(args[0], "quit") == 0) {
        return TRUE;
    } else if(strcmp(args[0], "info") == 0) {
        printf((board.turn == WHITE) ? "\nWhite to move\n" : "\nBlack to move\n");
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
        struct timeval tv;
        gettimeofday(&tv,NULL);
        unsigned long time_in_micros_1 = 1000000 * tv.tv_sec + tv.tv_usec;
        Moves moves = getLegalMoves();
        gettimeofday(&tv,NULL);
        unsigned long time_in_micros_2 = 1000000 * tv.tv_sec + tv.tv_usec;
        printf("Time: %ld\n", time_in_micros_2 - time_in_micros_1);
        
        printMoves(moves);
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
        printf("In check: %d", inCheck(board.turn, getPseudoLegalMoves((board.turn == WHITE) ? BLACK : WHITE)));
    } else if(strcmp(args[0], "rand") == 0) {
        int moves = atoi(args[1]);
        printf("Moves: %d", moves);
        for(int i = 0; i < moves; i ++) {
            makeRandomMove();    
        }
        printBoard();
    } else if(strcmp(args[0], "stress") == 0) {
        loadFENStr(args[2]);
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
            loadFENStr(args[2]);
            curr = curr->next;
        }
    } else if(strcmp(args[0], "perft") == 0) {
        printf("Nodes: %d\n", countMoves(atoi(args[1])));
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
        Moves moves = getPseudoLegalMoves((board.turn == WHITE) ? BLACK : WHITE);
        gettimeofday(&tv,NULL);
        unsigned long time_in_micros_2 = 1000000 * tv.tv_sec + tv.tv_usec;
        printf("Time: %ld\n", time_in_micros_2 - time_in_micros_1);
        
        printMoves(moves);
    }

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
