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
#define get(rank, file) board[rank*8+file]
#define set(rank, file, piece)board[rank*8+file] = piece

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

char board[64];

int enPassantTarget = 0;

int blackQCastleRight = TRUE;
int blackKCastleRight = TRUE;
int whiteQCastleRight = TRUE;
int whiteKCastleRight = TRUE;

int prevBlackQCastleRight = TRUE;
int prevBlackKCastleRight = TRUE;
int prevWhiteQCastleRight = TRUE;
int prevWhiteKCastleRight = TRUE;

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

int halfMoves = 0;
int fullMoves = 0;

MoveNode lastMove;
int prevEnPassantTarget;
char prevCapturedPiece;

char turn = 'w';

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

int getFile(char file) {
    return file - 97;
}

int getRank(char rank) {
    return rank - 49;
}

int parseSquare(char *square) {
    int rank = getRank(square[1]);
    int file = getFile(square[0]);

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

void addMove(Moves *moves, int origin, int dest) {
    if(origin >= 0 && origin < 64 && dest >= 0 && dest < 64) {
        MoveNode *node;

        node = (MoveNode *)calloc(1, sizeof(MoveNode));

        node->origin = origin;
        node->dest = dest;
        node->enPassantTarget = -1;
        node->castle = NO_CASTLE;
        node->promotion = NO_PROMOTION;

        if(moves->head == NULL) {
            moves->head = node;
            moves->tail = node;
        } else {
            moves->tail->next = node;
            moves->tail = node;
        }
        moves->len ++;
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
    MoveNode *node;

    node = (MoveNode *)calloc(1, sizeof(MoveNode));

    node->origin = origin;
    node->dest = dest;
    node->enPassantTarget = -1;
    node->castle = NO_CASTLE;
    node->promotion = promotion;
    if(moves->head == NULL) {
        moves->head = node;
        moves->tail = node;
    } else {
        moves->tail->next = node;
        moves->tail = node;
    }
    moves->len ++;
}

void addCastleMove(Moves *moves, int castle) {
    MoveNode *node;

    node = (MoveNode *)calloc(1, sizeof(MoveNode));

    node->origin = -1;
    node->dest = -1;
    node->enPassantTarget = -1;
    node->castle = castle;
    node->promotion = NO_PROMOTION;
    if(moves->head == NULL) {
        moves->head = node;
        moves->tail = node;
    } else {
        moves->tail->next = node;
        moves->tail = node;
    }
    moves->len ++;
}

void printMoves(Moves moves) {
    MoveNode *current = moves.head;
    printf("Len: %d\n", moves.len);
    while(current != NULL) {
        char origin[3];
        char dest[3];
        getSquareChar(current->origin, origin);
        getSquareChar(current->dest, dest);
        printf("%s %s %c \n", origin, dest, current->promotion);
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
                free(current);
                break;
            } else {
                last->next = current->next;
                if(last->next == NULL) positions->tail = last;
                free(current);
                break;
            }
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
                // printf("%c token = %d, literal = %d, argc = %d, pos = %d\n", ch, intoken, inliteral, *argc, pos);
            }
            else
            {
                if (intoken)
                {
                    intoken = FALSE;
                    inliteral = FALSE;
                    pos = 0;
                    // printf("%c token = %d, literal = %d, argc = %d, pos = %d\n", ch, intoken, inliteral, *argc, pos);
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
                // printf("%c token = %d, literal = %d, argc = %d, pos = %d\n", ch, intoken, inliteral, *argc, pos);
            }
            else
            {
                intoken = TRUE;
                inliteral = TRUE;
                *argc += 1;
                pos = 0;
                // printf("%c token = %d, literal = %d, argc = %d, pos = %d\n", ch, intoken, inliteral, *argc, pos);
            }
        }
        else if (ch >= 33 && ch <= 126)
        {

            if (intoken)
            {
                *(args[*argc] + pos) = ch;
                pos += 1;
                // printf("%c token = %d, literal = %d, argc = %d, pos = %d\n", ch, intoken, inliteral, *argc, pos);
            }
            else
            {
                *argc += 1;
                pos = 0;
                *(args[*argc] + pos) = ch;
                pos += 1;
                // printf("%c token = %d, literal = %d, argc = %d, pos = %d\n", ch, intoken, inliteral, *argc, pos);
            }
            intoken = TRUE;
        }
    }

    *argc += 1;
    return;
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

    
    clearPositions(&blackPawns);
    clearPositions(&blackKnights);
    clearPositions(&blackBishops);
    clearPositions(&blackRooks);
    clearPositions(&blackQueens);

    clearPositions(&whitePawns);
    clearPositions(&whiteKnights);
    clearPositions(&whiteBishops);
    clearPositions(&whiteRooks);
    clearPositions(&whiteQueens);
    

    while(FENCode[i] != ' ') {
        if(FENCode[i] == '/') {
            rank --;
            file = 0;
        } else if(FENCode[i] > 64) {
            set(rank, file, FENCode[i]);
            switch(FENCode[i]) {
                case 'p':
                    addPosition(&blackPawns, rank*8 + file);
                    break;
                case 'n':
                    addPosition(&blackKnights, rank*8 + file);
                    break;
                case 'b':
                    addPosition(&blackBishops, rank*8 + file);
                    break;
                case 'r':
                    addPosition(&blackRooks, rank*8 + file);
                    break;
                case 'q':
                    addPosition(&blackQueens, rank*8 + file);
                    break;
                case 'k':
                    blackKing = rank*8 + file;
                    break;
                case 'P':
                    addPosition(&whitePawns, rank*8 + file);
                    break;
                case 'N':
                    addPosition(&whiteKnights, rank*8 + file);
                    break;
                case 'B':
                    addPosition(&whiteBishops, rank*8 + file);
                    break;
                case 'R':
                    addPosition(&whiteRooks, rank*8 + file);
                    break;
                case 'Q':
                    addPosition(&whiteQueens, rank*8 + file);
                    break;
                case 'K':
                    whiteKing = rank*8 + file;
                    break;
            }
            file ++;
        } else {
            file += FENCode[i] - 48;
        }
        i++;
    }
    i++;
    turn = FENCode[i];
    i += 2;

    blackKCastleRight = FALSE;
    blackQCastleRight = FALSE;
    whiteKCastleRight = FALSE;
    whiteQCastleRight = FALSE;

    while(FENCode[i] != ' ') {
        if(FENCode[i] == 'Q') {
            whiteQCastleRight = TRUE;
        } else if(FENCode[i] == 'q') {
            blackQCastleRight = TRUE;
        } else if(FENCode[i] == 'K') {
            whiteKCastleRight = TRUE;
        } else if(FENCode[i] == 'k') {
            blackKCastleRight = TRUE;
        }
        i++;
    }
    i++;

    enPassantTarget = -1;
    if(FENCode[i] != '-') {
        enPassantTarget = getRank(FENCode[i+1])*8 + getFile(FENCode[i]);
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
    halfMoves = atoi(halfMovesChar);

    len = 0;
    while(FENCode[i] != '\0') {
        i ++;
        len ++;
    }
    char fullMovesChar[len];
    memcpy(fullMovesChar, &FENCode[i-len], len);
    fullMoves = atoi(fullMovesChar);
}

Positions *getPositionsOfType(char piece) {
    switch(piece) {
        case 'p':
            return &blackPawns;
        case 'n':
            return &blackKnights;
        case 'b':
            return &blackBishops;
        case 'r':
            return &blackRooks;
        case 'q':
            return &blackQueens;
        case 'P':
            return &whitePawns;
        case 'N':
            return &whiteKnights;
        case 'B':
            return &whiteBishops;
        case 'R':
            return &whiteRooks;
        case 'Q':
            return &whiteQueens;
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

void unMakeMove(MoveNode move, char capturedPiece, int lastEnPassantTarget, int changeTurn) {
    
    if(move.enPassantTarget != -1) {
        board[move.enPassantTarget] = capturedPiece;
        Positions *capturedPosition = getPositionsOfType(capturedPiece);
        addPosition(capturedPosition, move.enPassantTarget);

        Positions *movedPosition = getPositionsOfType(board[move.dest]);
        removePosition(movedPosition, move.dest);
        addPosition(movedPosition, move.origin);
        board[move.origin] = board[move.dest];
        board[move.dest] = ' ';
    } else if(move.castle != NO_CASTLE) {
        if(move.castle == WHITE_KING_SIDE_CASTLE) {
            board[7] = 'R';
            board[4] = 'K';
            board[6] = ' ';
            board[5] = ' ';
            whiteKing = 4;
            addPosition(&whiteRooks, 7);
            removePosition(&whiteRooks, 5);
        } else if(move.castle == WHITE_QUEEN_SIDE_CASTLE) {
            board[0] = 'R';
            board[4] = 'K';
            board[2] = ' ';
            board[3] = ' ';
            whiteKing = 4;
            addPosition(&whiteRooks, 0);
            removePosition(&whiteRooks, 3);
        } else if(move.castle == BLACK_KING_SIDE_CASTLE) {
            board[63] = 'r';
            board[60] = 'k';
            board[62] = ' ';
            board[61] = ' ';
            blackKing = 60;
            addPosition(&blackRooks, 63);
            removePosition(&blackRooks, 61);
        } else if(move.castle == BLACK_QUEEN_SIDE_CASTLE) {
            board[56] = 'r';
            board[60] = 'k';
            board[58] = ' ';
            board[59] = ' ';
            blackKing = 60;
            addPosition(&blackRooks, 56);
            removePosition(&blackRooks, 59);
        }
    } else {
        if(capturedPiece == 'K') {
            whiteKing = move.dest;
            board[move.dest] = 'K';
        } else if(capturedPiece == 'k') {
            board[move.dest] = 'k';
            blackKing = move.dest;
        } else if(capturedPiece != ' ') {
            Positions *capturedPosition = getPositionsOfType(capturedPiece);
            addPosition(capturedPosition, move.dest);
        }
        if(board[move.dest] == 'K') {
           whiteKing = move.origin;
           board[move.origin] = 'K';
           board[move.dest] = capturedPiece;
       } else if(board[move.dest] == 'k') {
           blackKing = move.origin;
           board[move.origin] = 'k';
           board[move.dest] = capturedPiece;
       } else {
            Positions *movedPosition = getPositionsOfType(board[move.dest]);
            removePosition(movedPosition, move.dest);
            addPosition(movedPosition, move.origin);

            board[move.origin] = board[move.dest];
            board[move.dest] = capturedPiece;
       }
    }

    if(move.promotion != NO_PROMOTION) {
        Positions *pawnPosition = (turn == WHITE) ? &whitePawns : &blackPawns;
        Positions *promotedPosition = getPositionsOfType(move.promotion);
        removePosition(promotedPosition, move.origin);
        addPosition(pawnPosition, move.origin);
        board[move.origin] = (turn == WHITE) ? 'P' : 'p';
    }

    enPassantTarget = lastEnPassantTarget;
    
    if(changeTurn) {
        if(turn == WHITE) turn = BLACK;
        else turn = WHITE;
    }
}

char makeMove(MoveNode move, int changeTurn) {
    char capture = ' ';
    enPassantTarget = -1;

    if(move.origin == 0) {
        whiteQCastleRight = FALSE;
    } else if(move.origin == 7) {
        whiteKCastleRight = FALSE;
    } else if(move.origin == 63) {
        blackKCastleRight = FALSE;
    } else if(move.origin == 56) {
        blackQCastleRight = FALSE;
    }

    if(move.enPassantTarget != -1) {
       Positions *movedPosition = getPositionsOfType(board[move.origin]);
       Positions *capturedPosition = getPositionsOfType(board[move.enPassantTarget]);
       removePosition(capturedPosition, move.enPassantTarget);
       removePosition(movedPosition, move.origin);
       addPosition(movedPosition, move.dest);

       board[move.dest] = board[move.origin];
       board[move.origin] = ' ';
       capture = board[move.enPassantTarget];
       board[move.enPassantTarget] = ' ';
    } else if(move.castle != NO_CASTLE) {
        if(move.castle == WHITE_KING_SIDE_CASTLE) {
            board[7] = ' ';
            board[4] = ' ';
            board[6] = 'K';
            board[5] = 'R';
            whiteKing = 6;
            whiteKCastleRight = FALSE;
            removePosition(&whiteRooks, 7);
            addPosition(&whiteRooks, 5);
        } else if(move.castle == WHITE_QUEEN_SIDE_CASTLE) {
            board[0] = ' ';
            board[4] = ' ';
            board[2] = 'K';
            board[3] = 'R';
            whiteKing = 2;
            whiteQCastleRight = FALSE;
            removePosition(&whiteRooks, 0);
            addPosition(&whiteRooks, 3);
        } else if(move.castle == BLACK_KING_SIDE_CASTLE) {
            board[63] = ' ';
            board[60] = ' ';
            board[62] = 'k';
            board[61] = 'r';
            blackKing = 62;
            blackKCastleRight = FALSE;
            removePosition(&blackRooks, 63);
            addPosition(&blackRooks, 61);
        } else if(move.castle == BLACK_QUEEN_SIDE_CASTLE) {
            board[56] = ' ';
            board[60] = ' ';
            board[58] = 'K';
            board[59] = 'R';
            blackKing = 58;
            blackQCastleRight = FALSE;
            removePosition(&blackRooks, 56);
            addPosition(&blackRooks, 59);
        }
    } else {
        if(board[move.dest] != ' ' && board[move.dest] != 'K' && board[move.dest] != 'k') {
            Positions *capturedPosition = getPositionsOfType(board[move.dest]);
            removePosition(capturedPosition, move.dest);
            capture = board[move.dest];
        } else {
            capture = board[move.dest];
        }
        if(board[move.origin] == 'K') {
            whiteKCastleRight = FALSE;
            whiteQCastleRight = FALSE;
            whiteKing = move.dest;
            capture = board[move.dest];
            board[move.dest] = 'K';
            board[move.origin] = ' ';  
        } else if(board[move.origin] == 'k') {
            blackKCastleRight = FALSE;
            blackQCastleRight = FALSE;
            blackKing = move.dest;
            capture = board[move.dest];
            board[move.dest] = 'k';
            board[move.origin] = ' ';
        } else {
            Positions *movedPosition = getPositionsOfType(board[move.origin]);
            removePosition(movedPosition, move.origin);
            addPosition(movedPosition, move.dest);

            board[move.dest] = board[move.origin];
            board[move.origin] = ' ';

            if((board[move.dest] == 'p' || board[move.dest] == 'P') && abs(move.dest - move.origin) == 16) {
                enPassantTarget = move.dest;
            }
        }
    }

    if(move.promotion != NO_PROMOTION) {
        Positions *pawnPosition = (turn == WHITE) ? &whitePawns : &blackPawns;
        Positions *promotedPosition = getPositionsOfType(move.promotion);
        addPosition(promotedPosition, move.dest);
        removePosition(pawnPosition, move.dest);
        board[move.dest] = move.promotion;
    }

    if(changeTurn) {
        if(turn == WHITE) turn = BLACK;
        else turn = WHITE;
    }

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
        MoveNode *node;

        node = (MoveNode *)calloc(1, sizeof(MoveNode));

        node->origin = origin;
        node->dest = dest;
        node->enPassantTarget = enPassantTarget;
        node->castle = NO_CASTLE;
        node->promotion = NO_PROMOTION;
        if(moves->head == NULL) {
            moves->head = node;
            moves->tail = node;
        } else {
            moves->tail->next = node;
            moves->tail = node;
        }
        moves->len ++;
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
        if(whiteKCastleRight && get(0, 5) == ' ' && get(0, 6) == ' ') {
            addCastleMove(&moves, WHITE_KING_SIDE_CASTLE);
        } else if(whiteQCastleRight && get(0, 3) == ' ' && get(0, 2) == ' ' && get(0, 1) == ' ') {
            addCastleMove(&moves, WHITE_QUEEN_SIDE_CASTLE);
        } 
    } else {
        if(blackKCastleRight && get(7, 5) == ' ' && get(7, 6) == ' ') {
            addCastleMove(&moves, BLACK_KING_SIDE_CASTLE);
        } else if(blackQCastleRight && get(7, 3) == ' ' && get(7, 2) == ' ' && get(7, 1) == ' ') {
            addCastleMove(&moves, BLACK_QUEEN_SIDE_CASTLE);
        }
    }

    int perspective = (colorToMove == WHITE) ? 1 : -1;
    //pawn moves
    PosNode *current = (colorToMove == WHITE) ? whitePawns.head : blackPawns.head;
    
    while(current != NULL) {
        if(board[current->pos] == ' ') {
            printf("Ghost pawn!!");
        }
        if(board[current->pos + perspective*8] == ' ') {
            if(current->pos/8 == (3.5-perspective*2.5) && board[current->pos + perspective*16] == ' ') {
                addPawnMove(&moves, current->pos, current->pos + perspective*16, colorToMove);
            }
            addPawnMove(&moves, current->pos, current->pos + perspective*8, colorToMove);
        }
        if(current->pos%8 != (3.5-perspective*3.5) && canTake(board[current->pos + perspective*7], colorToMove)) {
            addPawnMove(&moves, current->pos, current->pos + perspective*7, colorToMove);
        }

        if(current->pos%8 != (3.5+perspective*3.5) && canTake(board[current->pos + perspective*9], colorToMove)) {
            addPawnMove(&moves, current->pos, current->pos + perspective*9, colorToMove);
        }
        if(current->pos%8 != (3.5+perspective*3.5) && current->pos + perspective*1 == enPassantTarget) {
            addEnPassantMove(&moves, current->pos, current->pos + perspective*9);
        } else if(current->pos%8 != (3.5-perspective*3.5) && current->pos - perspective*1 == enPassantTarget) {
            addEnPassantMove(&moves, current->pos, current->pos + perspective*7);
        }
        current = current->next;
    }

    //knight moves
    current = (colorToMove == WHITE) ? whiteKnights.head : blackKnights.head;
    while(current != NULL) {
        if(board[current->pos] == ' ') {
            printf("Ghost knight!!");
        }
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

    current = (colorToMove == WHITE) ? whiteBishops.head : blackBishops.head;
    while(current != NULL) {
        if(board[current->pos] == ' ') {
            printf("Ghost bishop!!");
        }
        int file = current->pos%8;
        int rank = current->pos/8;
        int NEBlocked = FALSE;
        int SEBlocked = FALSE;
        int SWBlocked = FALSE;
        int NWBlocked = FALSE;

        for(int offset = 1; offset < 7; offset ++) {
            addMove3(&moves, current->pos, rank + offset, file + offset, &NEBlocked, colorToMove);
            addMove3(&moves, current->pos, rank - offset, file + offset, &SEBlocked, colorToMove);
            addMove3(&moves, current->pos, rank - offset, file - offset, &SWBlocked, colorToMove);
            addMove3(&moves, current->pos, rank + offset, file - offset, &NWBlocked, colorToMove);
        }
        
        current = current->next;
    }

    current = (colorToMove == WHITE) ? whiteRooks.head : blackRooks.head;
    while(current != NULL) {
        int file = current->pos%8;
        int rank = current->pos/8;
        if(board[current->pos] == ' ') {
            printf("Ghost rook!!");
        }
        
        int NBlocked = FALSE;
        int EBlocked = FALSE;
        int SBlocked = FALSE;
        int WBlocked = FALSE;
        
        for(int offset = 1; offset < 7; offset ++) {
            addMove3(&moves, current->pos, rank + offset, file, &NBlocked, colorToMove);
            addMove3(&moves, current->pos, rank, file + offset, &EBlocked, colorToMove);
            addMove3(&moves, current->pos, rank - offset, file, &SBlocked, colorToMove);
            addMove3(&moves, current->pos, rank, file - offset, &WBlocked, colorToMove);
        }

        current = current->next;
    }
    
    current = (colorToMove == WHITE) ? whiteQueens.head : blackQueens.head;
    while(current != NULL) {
        if(board[current->pos] == ' ') {
            printf("Ghost queen!!");
        }
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
        
        for(int offset = 1; offset < 7; offset ++) {
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

    int kingPos = (colorToMove == WHITE) ? whiteKing : blackKing;
    int kingRank = kingPos/8;
    int kingFile = kingPos%8;

    addMove2(&moves, kingPos, kingRank + 1, kingFile + 1, colorToMove);
    addMove2(&moves, kingPos, kingRank + 1, kingFile, colorToMove);
    addMove2(&moves, kingPos, kingRank + 1, kingFile - 1, colorToMove);
    addMove2(&moves, kingPos, kingRank + 0, kingFile + 1, colorToMove);
    addMove2(&moves, kingPos, kingRank + 0, kingFile, colorToMove);
    addMove2(&moves, kingPos, kingRank + 0, kingFile - 1, colorToMove);
    addMove2(&moves, kingPos, kingRank - 1, kingFile + 1, colorToMove);
    addMove2(&moves, kingPos, kingRank - 1, kingFile, colorToMove);
    addMove2(&moves, kingPos, kingRank - 1, kingFile - 1, colorToMove);

    return moves;
}

int inCheck(int player, Moves opponentMoves) {

    int kingPos;
    if(player == WHITE) {
        kingPos = whiteKing;
    } else {
        kingPos = blackKing;
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

    int lastEnPassantTarget = enPassantTarget;
    int lastWhiteKCastleRight = whiteKCastleRight;
    int lastWhiteQCastleRight = whiteQCastleRight;
    int lastBlackKCastleRight = blackKCastleRight;
    int lastBlackQCastleRight = blackQCastleRight;
    int lastCapture = makeMove(move, FALSE);
    Moves opponentMoves = getPseudoLegalMoves((turn == WHITE) ? BLACK : WHITE);
    int legal = TRUE;
    if(move.castle == NO_CASTLE) {
        legal = 1 - inCheck(turn, opponentMoves);
    } else {
        int safe[3];
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

    unMakeMove(move, lastCapture, lastEnPassantTarget, FALSE);
    whiteKCastleRight = lastWhiteKCastleRight;
    whiteQCastleRight = lastWhiteQCastleRight;
    blackKCastleRight = lastBlackKCastleRight;
    blackQCastleRight = lastBlackQCastleRight;

    return legal;
}

Moves getLegalMoves() {

    Moves pseudoLegalMoves = getPseudoLegalMoves(turn);
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
        move.castle = (turn == WHITE) ? WHITE_QUEEN_SIDE_CASTLE : BLACK_QUEEN_SIDE_CASTLE;
    } else if(strcmp(args[1], "O-O") == 0) {
        move.castle = (turn == WHITE) ? WHITE_KING_SIDE_CASTLE : BLACK_KING_SIDE_CASTLE;
        move.dest = parseSquare(args[3]);       
    } else if(strcmp(args[1], "promote") == 0) {
        move.origin = parseSquare(args[2]);
        move.dest = parseSquare(args[3]);
        move.promotion = args[4][0];
    } else {
        move.origin = parseSquare(args[1]);
        move.dest = parseSquare(args[2]);       
    }

    return move;
}

void makeRandomMove() {
    Moves moves = getLegalMoves(turn);
    if(moves.len != 0) {
        int index = (int)((rand() / (double)RAND_MAX) * moves.len);
        MoveNode *curr = moves.head;
        for(int i = 0; i < index; i ++) {
            curr = curr->next;
        }
        makeMove(*curr, TRUE);
    }
}

int execute(char *command) {

    int argc;
    char **args;

    args = init();
    parse(command, args, &argc);

    if(strcmp(args[0], "load") == 0) {
        loadFENStr(args[1]);
        printBoard();
    } else if(strcmp(args[0], "move") == 0) {

        prevBlackKCastleRight = blackKCastleRight;
        prevBlackQCastleRight = blackQCastleRight;
        prevWhiteKCastleRight = whiteKCastleRight;
        prevWhiteQCastleRight = whiteQCastleRight;

        Moves moves = getLegalMoves();
        MoveNode move = parseMove(args);
        if(containsMove(moves, move)) {
            lastMove = move;
            prevEnPassantTarget = enPassantTarget;
            prevCapturedPiece = makeMove(move, TRUE);
            printBoard();
        } else {
            printf("Illegal Move\n");
        }
        
    } else if(strcmp(args[0], "quit") == 0) {
        return TRUE;
    } else if(strcmp(args[0], "info") == 0) {
        printf((turn == WHITE) ? "\nWhite to move\n" : "\nBlack to move\n");
        printf("BlackKCastleRight: %d \n", blackKCastleRight);
        printf("BlackQCastleRight: %d \n", blackQCastleRight);
        printf("WhiteKCastleRight: %d \n", whiteKCastleRight);
        printf("WhiteQCastleRight: %d \n", whiteQCastleRight);
        printf("En Passant Target: %d \n", enPassantTarget);
        printf("Half Moves Completed: %d \n", halfMoves);
        printf("Full Move Clock: %d \n\n", fullMoves);
    } else if(strcmp(args[0], "board") == 0) {
        printBoard();
    } else if(strcmp(args[0], "moves") == 0) {
        struct timeval tv;
        gettimeofday(&tv,NULL);
        unsigned long time_in_micros_1 = 1000000 * tv.tv_sec + tv.tv_usec;
        Moves moves = getLegalMoves(turn);
        gettimeofday(&tv,NULL);
        unsigned long time_in_micros_2 = 1000000 * tv.tv_sec + tv.tv_usec;
        printf("Time: %ld\n", time_in_micros_2 - time_in_micros_1);
        
        printMoves(moves);
    } else if(strcmp(args[0], "positions") == 0) {

        printf("White \n");
        printf("Pawns: "); printPositions(whitePawns);
        printf("Knights: "); printPositions(whiteKnights);
        printf("Bishops: "); printPositions(whiteBishops);
        printf("Rooks: "); printPositions(whiteRooks);
        printf("Queens: "); printPositions(whiteQueens);
        
        printf("Black \n");
        printf("Pawns: "); printPositions(blackPawns);
        printf("Knights: "); printPositions(blackKnights);
        printf("Bishops: "); printPositions(blackBishops);
        printf("Rooks: "); printPositions(blackRooks);
        printf("Queens: "); printPositions(blackQueens);
    } else if(strcmp(args[0], "undo") == 0) {
        unMakeMove(lastMove, prevCapturedPiece, prevEnPassantTarget, TRUE);
        whiteKCastleRight = prevWhiteKCastleRight;
        whiteQCastleRight = prevWhiteQCastleRight;
        blackKCastleRight = prevBlackKCastleRight;
        blackQCastleRight = prevBlackQCastleRight;
        printBoard();
    } else if(strcmp(args[0], "check") == 0) {
        printf("In check: %d", inCheck(turn, getPseudoLegalMoves((turn == WHITE) ? BLACK : WHITE)));
    } else if(strcmp(args[0], "rand") == 0) {
        int moves = atoi(args[1]);
        printf("Moves: %d", moves);
        for(int i = 0; i < moves; i ++) {
            makeRandomMove();    
        }
        printBoard();
    } else if(strcmp(args[0], "stress") == 0) {
        Moves moves = getLegalMoves();
        int numMoves = atoi(args[1]);
        MoveNode *curr = moves.head;
        while(curr != NULL) {
            makeMove(*curr, TRUE);
            for(int i = 0; i < numMoves; i ++) {
                makeRandomMove();
            }
            printBoard();
            loadFENStr(STARTING_POS);
            curr = curr->next;
        }
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
