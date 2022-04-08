#include "Eval.h"

static inline int popCount(uint64_t *bitboard) {
    return __builtin_popcountll(*bitboard);
}

float randWeight() {
    return ((float)rand() / RAND_MAX) / 100.0 - 0.005;
}

float evaluate() {
    int whiteMaterial = 9*popCount(&queens(WHITE)) + 5*popCount(&rooks(WHITE)) + 3*popCount(&bishops(WHITE)) + 3*popCount(&knights(WHITE)) + popCount(&pawns(WHITE));
    int blackMaterial = 9*popCount(&queens(BLACK)) + 5*popCount(&rooks(BLACK)) + 3*popCount(&bishops(BLACK)) + 3*popCount(&knights(BLACK)) + popCount(&pawns(BLACK));
    return ((board_getTurn() == WHITE) ? (whiteMaterial - blackMaterial) : (blackMaterial - whiteMaterial)) + randWeight();
}