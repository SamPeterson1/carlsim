typedef struct MoveNode_s {

    struct MoveNode_s *next;
    int origin;
    int dest;
    int enPassantTarget;
    int castle;
    unsigned char promotion;

} MoveNode;

int fullyLegalMove(MoveNode node);