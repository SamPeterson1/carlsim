#include "MoveGenerator.h"

const uint64_t rookMagics[64] = {
    0xa8002c000108020ULL, 0x6c00049b0002001ULL, 0x100200010090040ULL, 0x2480041000800801ULL, 0x280028004000800ULL,
    0x900410008040022ULL, 0x280020001001080ULL, 0x2880002041000080ULL, 0xa000800080400034ULL, 0x4808020004000ULL,
    0x2290802004801000ULL, 0x411000d00100020ULL, 0x402800800040080ULL, 0xb000401004208ULL, 0x2409000100040200ULL,
    0x1002100004082ULL, 0x22878001e24000ULL, 0x1090810021004010ULL, 0x801030040200012ULL, 0x500808008001000ULL,
    0xa08018014000880ULL, 0x8000808004000200ULL, 0x201008080010200ULL, 0x801020000441091ULL, 0x800080204005ULL,
    0x1040200040100048ULL, 0x120200402082ULL, 0xd14880480100080ULL, 0x12040280080080ULL, 0x100040080020080ULL,
    0x9020010080800200ULL, 0x813241200148449ULL, 0x491604001800080ULL, 0x100401000402001ULL, 0x4820010021001040ULL,
    0x400402202000812ULL, 0x209009005000802ULL, 0x810800601800400ULL, 0x4301083214000150ULL, 0x204026458e001401ULL,
    0x40204000808000ULL, 0x8001008040010020ULL, 0x8410820820420010ULL, 0x1003001000090020ULL, 0x804040008008080ULL,
    0x12000810020004ULL, 0x1000100200040208ULL, 0x430000a044020001ULL, 0x280009023410300ULL, 0xe0100040002240ULL,
    0x200100401700ULL, 0x2244100408008080ULL, 0x8000400801980ULL, 0x2000810040200ULL, 0x8010100228810400ULL,
    0x2000009044210200ULL, 0x4080008040102101ULL, 0x40002080411d01ULL, 0x2005524060000901ULL, 0x502001008400422ULL,
    0x489a000810200402ULL, 0x1004400080a13ULL, 0x4000011008020084ULL, 0x26002114058042ULL
};

const uint64_t bishopMagics[64] = {
    0x89a1121896040240ULL, 0x2004844802002010ULL, 0x2068080051921000ULL, 0x62880a0220200808ULL, 0x4042004000000ULL,
    0x100822020200011ULL, 0xc00444222012000aULL, 0x28808801216001ULL, 0x400492088408100ULL, 0x201c401040c0084ULL,
    0x840800910a0010ULL, 0x82080240060ULL, 0x2000840504006000ULL, 0x30010c4108405004ULL, 0x1008005410080802ULL,
    0x8144042209100900ULL, 0x208081020014400ULL, 0x4800201208ca00ULL, 0xf18140408012008ULL, 0x1004002802102001ULL,
    0x841000820080811ULL, 0x40200200a42008ULL, 0x800054042000ULL, 0x88010400410c9000ULL, 0x520040470104290ULL,
    0x1004040051500081ULL, 0x2002081833080021ULL, 0x400c00c010142ULL, 0x941408200c002000ULL, 0x658810000806011ULL,
    0x188071040440a00ULL, 0x4800404002011c00ULL, 0x104442040404200ULL, 0x511080202091021ULL, 0x4022401120400ULL,
    0x80c0040400080120ULL, 0x8040010040820802ULL, 0x480810700020090ULL, 0x102008e00040242ULL, 0x809005202050100ULL,
    0x8002024220104080ULL, 0x431008804142000ULL, 0x19001802081400ULL, 0x200014208040080ULL, 0x3308082008200100ULL,
    0x41010500040c020ULL, 0x4012020c04210308ULL, 0x208220a202004080ULL, 0x111040120082000ULL, 0x6803040141280a00ULL,
    0x2101004202410000ULL, 0x8200000041108022ULL, 0x21082088000ULL, 0x2410204010040ULL, 0x40100400809000ULL,
    0x822088220820214ULL, 0x40808090012004ULL, 0x910224040218c9ULL, 0x402814422015008ULL, 0x90014004842410ULL,
    0x1000042304105ULL, 0x10008830412a00ULL, 0x2520081090008908ULL, 0x40102000a0a60140ULL,
};

const int rookIndexBits[64] = {
    12, 11, 11, 11, 11, 11, 11, 12,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    12, 11, 11, 11, 11, 11, 11, 12
};

const int bishopIndexBits[64] = {
    6, 5, 5, 5, 5, 5, 5, 6,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    6, 5, 5, 5, 5, 5, 5, 6
};

const int directionOffsets[8][2] = {
    {1, 0}, {1, 1}, {0, 1}, {-1, 1}, {-1, 0}, {-1, -1}, {0, -1}, {1, -1}
};

const int slidingDirections[4] = {
    NORTH, EAST, SOUTH, WEST
};

const int diagonalDirections[4] = {
    NORTHEAST, SOUTHEAST, SOUTHWEST, NORTHWEST
};

const int knightOffsets[8][2] = {
    {2, 1}, {1, 2}, {-2, 1}, {-1, 2}, {2, -1}, {1, -2}, {-2, -1}, {-1, -2}
};

const uint64_t edgeMask = 0x00818181818181FF;

unsigned long totalTime = 0;
unsigned long movesGenerated = 0;
uint64_t enPassantCaptures[8][4]; //[en passant file][pawns on epFile+1, epFile, and epFile-1]
uint64_t pawnAttacks[64][2];
uint64_t leftPawnAttacks[64][2];
uint64_t rightPawnAttacks[64][2];
uint64_t pawnAdvances[64][2];
uint64_t knightAttacks[64];
uint64_t bishopMoves[64];
uint64_t rookMoves[64];
uint64_t directionalMoves[64][8];
uint64_t kingMoves[64];

MagicHashTable *rookMagicTables[64];
MagicHashTable *bishopMagicTables[64];

uint64_t rankMasks[8];

uint64_t attackedSquares;
uint64_t checkers;
uint64_t checkMask;
uint64_t pinRays[64];

//add 7 to distance
int positionToDirectionMap[64][64];

int numCheckers;

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

unsigned long micros() {
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return 1000000 * tv.tv_sec + tv.tv_usec;
}

/*
static inline void generateKnightAttacks(uint64_t *moves, int turnIndex) {
    uint64_t knights = knights(turnIndex);
    while(knights) {
        *moves |= knightAttacks[popLSB(&knights)];
    }
}
*/

/*
static inline void generatePawnAttacks(uint64_t *moves, int turnIndex) {
    uint64_t pawns = pawns(turnIndex);
    while(pawns) {
        int pos = popLSB(&pawns);
        *moves |= pawnAttacks[pos][turnIndex];
    }
}
*/

void printBitboard(uint64_t *bitboard) {
    for(int rank = 7; rank >= 0; rank --) {
        for(int file = 0; file < 8; file ++) {
            int i = rank*8 + file;
            if(((*bitboard >> i) & 1) == 1) printf("1 ");
            else printf(". ");
        }
        printf("\n");
    }
    printf("\n");
}

int inBounds(int rank, int file) {
    return (rank >= 0 & rank < 8 && file >= 0 && file < 8);
}

int notOnEdge(int rank, int file) {
    return (rank > 0 & rank < 7 && file > 0 && file < 7);
}

void addMove(uint64_t *bitboard, int rank, int file) {
    if(inBounds(rank, file)) {
        *bitboard |= 1ULL << (rank*8 + file);
    }
}

void generateRookMoves(int index, uint64_t *blockers, uint64_t *moves) {
    int rank = (int) (index / 8);
    int file = index % 8;
    for(int dirIndex = 0; dirIndex < 4; dirIndex ++) {   
        int dir = slidingDirections[dirIndex];      
        for(int offset = 1; offset < 8; offset ++) {
            int newRank = rank + directionOffsets[dir][0]*offset;
            int newFile = file + directionOffsets[dir][1]*offset;
            if(!inBounds(newRank, newFile)) break;
            addMove(moves, newRank, newFile);
            int index = newRank * 8 + newFile;
            if(((*blockers >> index) & 1) == 1) break;
        }
    }
}

void generateBishopMoves(int index, uint64_t *blockers, uint64_t *moves) {
    int rank = (int) (index / 8);
    int file = index % 8;
    for(int dirIndex = 0; dirIndex < 4; dirIndex ++) {        
        int dir = diagonalDirections[dirIndex]; 
        for(int offset = 1; offset < 8; offset ++) {
            int newRank = rank + directionOffsets[dir][0]*offset;
            int newFile = file + directionOffsets[dir][1]*offset;      
            if(!inBounds(newRank, newFile)) break;
            addMove(moves, newRank, newFile);
            int index = newRank * 8 + newFile;
            if(((*blockers >> index) & 1) == 1) break;
        }
    }
}

MagicHashTable *generateRookAttackTable(int index, uint64_t *rookMoves) {
    int rank = (int) (index / 8);
    int file = index % 8;

    MagicHashTable *rookAttackTable = createTable(1 << rookIndexBits[index], rookIndexBits[index], rookMagics[index]);
    uint64_t rookAttacks = 0;
    int multipliers[16];
    int multPtr = 0;
    for(int dirIndex = 0; dirIndex < 4; dirIndex ++) {
        uint64_t directionalRookAttacks = 0;
        int dir = slidingDirections[dirIndex];
        for(int offset = 1; offset < 7; offset ++) {
            int newRank = rank + directionOffsets[dir][0]*offset;
            int newFile = file + directionOffsets[dir][1]*offset;
            int testRank = rank + directionOffsets[dir][0]*(offset + 1);
            int testFile = file + directionOffsets[dir][1]*(offset + 1);
            if(inBounds(testRank, testFile)) {
                int index = 8*newRank + newFile;
                rookAttacks |= 1ULL << index;
                multipliers[multPtr] = index;
                multPtr ++;
                addMove(&directionalRookAttacks, newRank, newFile);
            }
        }
        directionalMoves[index][dir] = directionalRookAttacks;
    } 

    *rookMoves = rookAttacks;
    for(int i = 0; i < (1 << multPtr); i ++) {
        uint64_t blockers = 0;
        for(int j = 0; j < multPtr; j ++) {
            blockers |= (uint64_t) ((i >> j) & 1) << multipliers[j];
        }
        uint64_t moves = 0;
        generateRookMoves(index, &blockers, &moves);
        insertValue(rookAttackTable, &blockers, &moves);
    }

    return rookAttackTable;
}

MagicHashTable *generateBishopAttackTable(int index, uint64_t *bishopMoves) {
    int rank = (int) (index / 8);
    int file = index % 8;

    MagicHashTable *bishopAttackTable = createTable(1 << bishopIndexBits[index], bishopIndexBits[index], bishopMagics[index]);
    uint64_t bishopAttacks = 0;
    int multipliers[16];
    int multPtr = 0;
    for(int dirIndex = 0; dirIndex < 4; dirIndex ++) {
        uint64_t directionalBishopAttacks = 0;
        int dir = diagonalDirections[dirIndex];
        for(int offset = 1; offset < 7; offset ++) {
            int newRank = rank + directionOffsets[dir][0]*offset;
            int newFile = file + directionOffsets[dir][1]*offset;
            int testRank = rank + directionOffsets[dir][0]*(offset + 1);
            int testFile = file + directionOffsets[dir][1]*(offset + 1);
            if(inBounds(testRank, testFile)) {
                int index = 8*newRank + newFile;
                bishopAttacks |= 1ULL << index;
                multipliers[multPtr] = index;
                multPtr ++;
                addMove(&directionalBishopAttacks, newRank, newFile);
            }
        }

        directionalMoves[index][dir] = directionalBishopAttacks;
    } 

    *bishopMoves = bishopAttacks;
    for(int i = 0; i < (1 << multPtr); i ++) {
        uint64_t blockers = 0;
        for(int j = 0; j < multPtr; j ++) {
            blockers |= (uint64_t) ((i >> j) & 1) << multipliers[j];
        }

        uint64_t moves = 0; 
        generateBishopMoves(index, &blockers, &moves);
        insertValue(bishopAttackTable, &blockers, &moves);
    }

    return bishopAttackTable;
}

void initMoveGenerator() {

    for(int rank = 0; rank < 8; rank ++) {
        rankMasks[rank] = 255ULL << rank*8;
        for(int file = 0; file < 8; file ++) {
            int index = rank*8 + file;
            for(int destRank = 0; destRank < 8; destRank ++) {
                for(int destFile = 0; destFile < 8; destFile ++) {
                    int destIndex = destRank*8 + destFile;
                    int dRank = rank - destRank;
                    int dFile = file - destFile;
                    int dir = NONE;

                    if(dFile == 0) {
                        if(dRank > 0) dir = SOUTH;
                        else if(dRank < 0) dir = NORTH;
                    } else if(dRank == 0) {
                        if(dFile > 0) dir = WEST;
                        else if(dFile < 0) dir = EAST;
                    } else if(abs(dRank) == abs(dFile)) {
                        if(dRank > 0 && dFile > 0) dir = SOUTHWEST;
                        else if(dRank < 0 && dFile > 0) dir = NORTHWEST;
                        else if(dRank < 0 && dFile < 0) dir = NORTHEAST;
                        else if(dRank > 0 && dFile < 0) dir = SOUTHEAST;
                    }

                    uint64_t a = (1ULL << index) | (1ULL << destIndex);                    
                    positionToDirectionMap[index][destIndex] = dir;
                }
            }
           
            if(file != 0) {
                if(rank < 7) pawnAttacks[index][WHITE] |= 1ULL << (index + 7);
                if(rank > 0) pawnAttacks[index][BLACK] |= 1ULL << (index - 9);
                if(rank < 6) leftPawnAttacks[index][WHITE] = 1ULL << (index + 7);
                if(rank > 1) leftPawnAttacks[index][BLACK] = 1ULL << (index - 9);
            }
            if(file != 7) {
                if(rank < 7) pawnAttacks[index][WHITE] |= 1ULL << (index + 9);
                if(rank > 0) pawnAttacks[index][BLACK] |= 1ULL << (index - 7); 
                if(rank < 6) rightPawnAttacks[index][WHITE] = 1ULL << (index + 9);
                if(rank > 1) rightPawnAttacks[index][BLACK] = 1ULL << (index - 7);
            }

            pawnAdvances[index][WHITE] |= 1ULL << (index + 8);
            pawnAdvances[index][BLACK] |= 1ULL << (index - 8);

            addMove(&knightAttacks[index], rank + 2, file + 1);
            addMove(&knightAttacks[index], rank + 1, file + 2);
            addMove(&knightAttacks[index], rank - 2, file + 1);
            addMove(&knightAttacks[index], rank - 1, file + 2);
            addMove(&knightAttacks[index], rank + 2, file - 1);
            addMove(&knightAttacks[index], rank + 1, file - 2);
            addMove(&knightAttacks[index], rank - 2, file - 1);
            addMove(&knightAttacks[index], rank - 1, file - 2);

            addMove(&kingMoves[index], rank + 1, file + 1);
            addMove(&kingMoves[index], rank - 1, file + 1);
            addMove(&kingMoves[index], rank, file + 1);
            addMove(&kingMoves[index], rank + 1, file - 1);
            addMove(&kingMoves[index], rank - 1, file - 1);
            addMove(&kingMoves[index], rank, file - 1);
            addMove(&kingMoves[index], rank + 1, file);
            addMove(&kingMoves[index], rank - 1, file);

            bishopMagicTables[index] = generateBishopAttackTable(index, &bishopMoves[index]);
            rookMagicTables[index] = generateRookAttackTable(index, &rookMoves[index]);
        }
    }
}

/*
static inline void generateDiagonalAttacks(uint64_t *moves, int index) {
    
}
*/

/*
static inline void generateSlidingAttacks(uint64_t *moves, int index) {
    uint64_t kingBitboard = kings(board_getTurn());
    int kingPos = getLSB(&kingBitboard);

    uint64_t blockers = rookMoves[index] & (board.bitboard & ~kingBitboard);
    uint64_t move = *GET_VALUE(rookMagicTables[index], &blockers);
    *moves |= move;
    
    int dir = positionToDirectionMap[index][kingPos];
    if(dir != NONE) {
        uint64_t kingBlocker = kingBitboard & directionalMoves[index][dir];
        uint64_t movesToKing = (directionalMoves[index][dir] & *GET_VALUE(rookMagicTables[index], &kingBlocker)) & ~kingBitboard;
        uint64_t kingGuards = board.bitboard & movesToKing;
        if(move & kingBitboard) {
            checkMask = movesToKing | (1ULL << index);
        } else if(popCount(kingGuards) == 1) {
            int pinIndex = getLSB(&kingGuards);
            pinRays[pinIndex] = movesToKing | (1ULL << index);
        }
    }
}
*/

static inline unsigned long perftRecursive(int depth) {
    uint16_t *moves = malloc(sizeof(uint16_t)*255);
    int numMoves = generateLegalMoves(moves);
    movesGenerated += numMoves;
    unsigned long sum = 0;
    if(depth == 1) {
        free(moves);
        return numMoves;
    } else {
        for(int i = 0; i < numMoves; i ++) {
            uint16_t lastGameState = board.gameState;
            unsigned char lastCapture = makeMove(moves[i]);
            sum += perftRecursive(depth-1);
            unMakeMove(moves[i], lastCapture, lastGameState);
        }
    }
    free(moves);
    return sum;
}

void perft(int depth) {
    totalTime = 0;
    movesGenerated = 0;
    uint16_t *moves = malloc(sizeof(uint16_t)*255);
    int numMoves = generateLegalMoves(moves);
    movesGenerated += numMoves;
    unsigned long total = 0;
    for(int i = 0; i < numMoves; i ++) {
        char moveStr[7];
        toStr(moves[i], moveStr);
        if(depth == 1) {
            printf("%s: 1\n", moveStr);
            total ++;
        } else {
            uint16_t lastGameState = board.gameState;
            unsigned char lastCapture = makeMove(moves[i]);
            int val = perftRecursive(depth-1);
            total += val;
            printf("%s: %d %d\n", moveStr, val, move_getSpecial(moves[i]));
            unMakeMove(moves[i], lastCapture, lastGameState);
        } 
    }
    free(moves);
    printf("\nTotal nodes searched: %ld\n", total);
    printf("Average move generation time: %f moves/sec\n", (double)movesGenerated/totalTime*1.0E6);
}

int generateLegalMoves(uint16_t *moves) {
    unsigned long foo = micros();
    int turn = board_getTurn();
    int oppTurn = 1 - turn;
    uint64_t kingBitboard = kings(board_getTurn());
    int kingPos = getLSB(&kingBitboard);
    
    attackedSquares = 0;    
    numCheckers = 0;
    checkers = 0;
    checkMask = 0;
    
    memset(pinRays, 255, 64*8);

    uint64_t knights = knights(oppTurn);
    while(knights) {
        attackedSquares |= knightAttacks[popLSB(&knights)];
    }

    uint64_t pawns = pawns(oppTurn);
    while(pawns) {
        attackedSquares |= pawnAttacks[popLSB(&pawns)][oppTurn];
    }

    uint64_t diagonals = bishops(oppTurn) | queens(oppTurn);
    while(diagonals) {
        int index = popLSB(&diagonals);
        uint64_t blockers = bishopMoves[index] & (board.bitboard & ~kingBitboard);
        uint64_t move = *GET_VALUE(bishopMagicTables[index], &blockers);
        attackedSquares |= move;
        
        int dir = positionToDirectionMap[index][kingPos];
        if(dir != NONE) {
            uint64_t kingBlocker = kingBitboard & directionalMoves[index][dir];
            uint64_t movesToKing = (directionalMoves[index][dir] & *GET_VALUE(bishopMagicTables[index], &kingBlocker)) & ~kingBitboard;
            uint64_t kingGuards = board.bitboard & movesToKing;
            if(move & kingBitboard) {
                checkMask = movesToKing | (1ULL << index);
            } else if(popCount(kingGuards) == 1) {
                int pinIndex = getLSB(&kingGuards);
                pinRays[pinIndex] = movesToKing | (1ULL << index);
            }
        }
    }
    
    uint64_t sliders = rooks(oppTurn) | queens(oppTurn);
    while(sliders) {   
        int index = popLSB(&sliders);
        uint64_t blockers = rookMoves[index] & (board.bitboard & ~kingBitboard);
        uint64_t move = *GET_VALUE(rookMagicTables[index], &blockers);
        attackedSquares |= move;
        
        int dir = positionToDirectionMap[index][kingPos];
        if(dir != NONE) {
            uint64_t kingBlocker = kingBitboard & directionalMoves[index][dir];
            uint64_t movesToKing = (directionalMoves[index][dir] & *GET_VALUE(rookMagicTables[index], &kingBlocker)) & ~kingBitboard;
            uint64_t kingGuards = board.bitboard & movesToKing;
            if(move & kingBitboard) {
                checkMask = movesToKing | (1ULL << index);
            } else if(popCount(kingGuards) == 1) {
                int pinIndex = getLSB(&kingGuards);
                pinRays[pinIndex] = movesToKing | (1ULL << index);
            }
        }
    }
    
    int opponentKingPos = getLSB(&kings(oppTurn));
    attackedSquares |= kingMoves[opponentKingPos] & ~attackedSquares;
    if(((attackedSquares >> kingPos) & 1) == 1) {
        uint64_t pawnAndKnightCheckers = (pawnAttacks[kingPos][turn] & pawns(oppTurn)) | (knightAttacks[kingPos] & knights(oppTurn));
        checkers |= pawnAndKnightCheckers;
        checkMask |= pawnAndKnightCheckers;
        uint64_t blockers = bishopMoves[kingPos] & board.bitboard;
        uint64_t diagonalAttacks = *GET_VALUE(bishopMagicTables[kingPos], &blockers);
        blockers = rookMoves[kingPos] & board.bitboard;
        uint64_t slidingAttacks = *GET_VALUE(rookMagicTables[kingPos], &blockers);
        checkers |= diagonalAttacks & (bishops(oppTurn) | queens(oppTurn));
        checkers |= slidingAttacks & (rooks(oppTurn) | queens(oppTurn));
        numCheckers = popCount(checkers);
    }
    if(numCheckers == 0) {
        memset(&checkMask, 255, 8);
    }

    int numMoves = 0;
    uint64_t kingMovesBitboard = kingMoves[kingPos] & ~(attackedSquares | board.colorBitboards[turn]);
    while(kingMovesBitboard) {
        moves[numMoves++] = create_move(kingPos, popLSB(&kingMovesBitboard), MOVE_QUIET);
    }

    if(numCheckers == 2) return numMoves;

    pawns = pawns(turn);
    int pushDirection = 1 - 2*turn;
    int pushRank = 1 + turn*5;
    int epRank = 4 - turn;

    while(pawns) {
        int pos = popLSB(&pawns);
        uint64_t movesBitboard = pawnAttacks[pos][turn] & board.colorBitboards[1-turn];
        movesBitboard |= pawnAdvances[pos][turn] & (~board.bitboard);

        int rank = pos / 8;
        if(rank == pushRank && ((board.bitboard >> (pos + pushDirection*8)) & 1) == 0) {
            movesBitboard |= (~board.bitboard) & (1ULL << (pos + pushDirection*16));
        }

        int epFile = board_getEPFile();
        if(epFile >= 0 && rank == epRank) {
            int file = pos & 7;
            int canTake = TRUE;

            uint64_t slidingPiecesOnRank = (queens(oppTurn) | rooks(oppTurn)) & rankMasks[rank];

            if((kings(turn) & rankMasks[rank]) != 0) {
                while(slidingPiecesOnRank) {
                    int slidingPiece = popLSB(&slidingPiecesOnRank);
                    int dir = positionToDirectionMap[slidingPiece][getLSB(&kings(turn))];
                    uint64_t kingBlocker = kings(turn) & directionalMoves[slidingPiece][dir];
                    uint64_t movesToKing = (directionalMoves[slidingPiece][dir] & *GET_VALUE(rookMagicTables[slidingPiece], &kingBlocker)) & ~kings(turn);
                    uint64_t kingGuards = board.bitboard & movesToKing;
                    if(popCount(kingGuards) == 2) {
                        canTake = FALSE; 
                        break;
                    }
                }
            }

            if(canTake) {
                uint64_t epBitboard = 0;
                if(file + 1 == epFile) {
                    uint64_t epDest = rightPawnAttacks[pos][turn];
                    int epCapture = getLSB(&epDest) + 8*(2*turn-1);
                    if(((checkers >> epCapture) & 1) == 1) {
                        epBitboard |= rightPawnAttacks[pos][turn] & pinRays[pos];
                    } else {
                        epBitboard |= rightPawnAttacks[pos][turn] & pinRays[pos] & checkMask;
                    }   
                } else if(file - 1 == epFile) {
                    uint64_t epDest = leftPawnAttacks[pos][turn];
                    int epCapture = getLSB(&epDest) + 8*(2*turn-1);
                    if(((checkers >> epCapture) & 1) == 1) {
                        epBitboard |= leftPawnAttacks[pos][turn] & pinRays[pos];
                    } else {
                        epBitboard |= leftPawnAttacks[pos][turn] & pinRays[pos] & checkMask;
                    }
                }

                while(epBitboard) {
                    moves[numMoves++] = create_move(pos, popLSB(&epBitboard), MOVE_EP_CAPTURE);
                }
            }
        }

        movesBitboard &= pinRays[pos] & checkMask;
        while(movesBitboard) {
            int dest = popLSB(&movesBitboard);
            int destRank = dest / 8;
            if(destRank == 0 || destRank == 7) {
                moves[numMoves++] = create_move(pos, dest, MOVE_KNIGHT_PROMOTION);
                moves[numMoves++] = create_move(pos, dest, MOVE_BISHOP_PROMOTION);
                moves[numMoves++] = create_move(pos, dest, MOVE_ROOK_PROMOTION);
                moves[numMoves++] = create_move(pos, dest, MOVE_QUEEN_PROMOTION);
            } else {
                moves[numMoves++] = create_move(pos, dest, MOVE_QUIET);
            }
        }
    }

    knights = knights(turn);
    while(knights) {
        
        int pos = popLSB(&knights);
        uint64_t movesBitboard = knightAttacks[pos] & ~board.colorBitboards[turn] & pinRays[pos] & checkMask;
        while(movesBitboard) {
            moves[numMoves++] = create_move(pos, popLSB(&movesBitboard), MOVE_QUIET);
        }
    }

    diagonals = bishops(turn) | queens(turn);
    while(diagonals) {
        int pos = popLSB(&diagonals);
        uint64_t blockers = bishopMoves[pos] & board.bitboard;
        uint64_t movesBitboard = *GET_VALUE(bishopMagicTables[pos], &blockers) & ~board.colorBitboards[turn] & pinRays[pos] & checkMask;
        
        while(movesBitboard) {
            moves[numMoves++] = create_move(pos, popLSB(&movesBitboard), MOVE_QUIET);
        }     
    }
    
    sliders = rooks(turn) | queens(turn);
    while(sliders) {
        int pos = popLSB(&sliders);
        uint64_t blockers = rookMoves[pos] & board.bitboard;
        uint64_t movesBitboard = *GET_VALUE(rookMagicTables[pos], &blockers) & ~board.colorBitboards[turn] & pinRays[pos] & checkMask;
        while(movesBitboard) {
            moves[numMoves++] = create_move(pos, popLSB(&movesBitboard), MOVE_QUIET);
        }
    }
    
    if(checkers == 0) {
        uint64_t castleCheck = (attackedSquares | board.bitboard) ^ kings(turn);
        if(turn == WHITE) {
            if(board_hasCastleRight(WHITE_CASTLE_KINGSIDE_RIGHT) && (castleCheck & WHITE_KINGSIDE_CASTLE_MASK) == 0 && board.pieceCodes[6] == EMPTY) {
                moves[numMoves++] = create_move(kingPos, kingPos + 2, MOVE_KINGSIDE_CASTLE);
            }
            if(board_hasCastleRight(WHITE_CASTLE_QUEENSIDE_RIGHT) && (castleCheck & WHITE_QUEENSIDE_CASTLE_MASK) == 0 && board.pieceCodes[1] == EMPTY) {
                moves[numMoves++] = create_move(kingPos, kingPos - 2, MOVE_QUEENSIDE_CASTLE);
            }
        } else if(turn == BLACK) {
            if(board_hasCastleRight(BLACK_CASTLE_KINGSIDE_RIGHT) && (castleCheck & BLACK_KINGSIDE_CASTLE_MASK) == 0 && board.pieceCodes[62] == EMPTY) {
                moves[numMoves++] = create_move(kingPos, kingPos + 2, MOVE_KINGSIDE_CASTLE);
            }
            if(board_hasCastleRight(BLACK_CASTLE_QUEENSIDE_RIGHT) && (castleCheck & BLACK_QUEENSIDE_CASTLE_MASK) == 0 && board.pieceCodes[57] == EMPTY) {
                moves[numMoves++] = create_move(kingPos, kingPos - 2, MOVE_QUEENSIDE_CASTLE);
            }
        }
    }

    totalTime += micros() - foo;
    return numMoves;
}

void test() {

}