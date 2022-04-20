#ifndef DEFINES_H

#define DEFINES_H

#define TRUE 1
#define FALSE 0

#define STARTING_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

#define EMPTY 255

#define WHITE 0
#define BLACK 1

#define PAWN 0
#define KNIGHT 2
#define BISHOP 4
#define ROOK 6
#define QUEEN 8
#define KING 10

#define WHITE_CASTLE_QUEENSIDE_RIGHT 1
#define WHITE_CASTLE_KINGSIDE_RIGHT 2
#define BLACK_CASTLE_QUEENSIDE_RIGHT 4
#define BLACK_CASTLE_KINGSIDE_RIGHT 8

#define NO_MATE 0
#define STALE_MATE 1
#define CHECK_MATE 2

#define Z_MAX 0xFFFFFFFFFFFFFFFF;
#define Z_KEY uint64_t
#define PAWN_VALUE 71
#define KNIGHT_VALUE 293
#define BISHOP_VALUE 300
#define ROOK_VALUE 456
#define QUEEN_VALUE 905
#define EVAL_INF 65535

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#define VALUE(piece) pieceValues[(piece) >> 1]

#define TABLE_SIZE (1 << 16)
#define RANK_8_MASK 0xfe00000000000000ULL
#define RANK_1_MASK 0xffULL
#define EDGE_MASK 0x7e7e7e7e7e7e00

#define WHITE_KINGSIDE_CASTLE_MASK 0x70
#define WHITE_QUEENSIDE_CASTLE_MASK 0x1C
#define BLACK_KINGSIDE_CASTLE_MASK 0x7000000000000000
#define BLACK_QUEENSIDE_CASTLE_MASK 0x1c00000000000000

#define NORTH 0
#define NORTHEAST 1
#define EAST 2
#define SOUTHEAST 3
#define SOUTH 4
#define SOUTHWEST 5
#define WEST 6
#define NORTHWEST 7
#define NONE 8

#define GEN_ALL 0
#define GEN_CAPTURES 1
#define GEN_QUIETS 2

#define GET_VALUE(t, k) (( t )->items[((*( k )) * ( t )->magic) >> (uint64_t)(64-( t )->bits)])->value

#define MOVE_QUIET 0
#define MOVE_DOUBLE_PAWN_PUSH 0x1000

#define MOVE_KINGSIDE_CASTLE 0x2000
#define MOVE_QUEENSIDE_CASTLE 0x3000

#define MOVE_CAPTURE 0x4000
#define MOVE_EP_CAPTURE 0x5000

#define MOVE_PROMOTION 0x8000
#define MOVE_KNIGHT_PROMOTION 0x8000
#define MOVE_BISHOP_PROMOTION 0x9000
#define MOVE_ROOK_PROMOTION 0xA000
#define MOVE_QUEEN_PROMOTION 0xB000

#define move_origin(move) (move & 0x3F)
#define move_dest(move) ((move & 0xFC0) >> 6)
#define move_isCapture(move) ((move >> 14) & 1)
#define move_isPromotion(move) ((move >> 15) & 1)
#define move_getSpecial(move) (move & 0xF000)

#define move_setOrigin(move, origin) move |= origin
#define move_setDest(move, dest) move |= (dest << 6)
#define move_setCapture(move, capture) move |= (capture << 13)
#define move_setPromotion(move, promotion) move |= (promotion << 14)d
#define move_setSpecial(move, special) move |= special

#define create_move(origin, dest, special) ((origin) | ((dest) << 6) | (special))

#define SEARCH_DEPTH 16

#define PIECE_TYPE(piece) (piece & 0xE)
#define PIECE_COLOR(piece) (piece & 1)

#define G_TURN ((g_board.gameState >> 8) & 1)
#define G_OPP_TURN (1 - TURN)
#define G_HAS_CASTLE_RIGHT(castle) ((g_board.gameState & castle) == castle)
#define G_EP_FILE (((g_board.gameState >> 4) & 0x0F) - 1)
#define G_HALFMOVE_COUNTER (g_board.gameState >> 8)
#define G_SET_EP_FILE(EPFile) g_board.gameState = ((g_board.gameState & 0xFF0F) | (((EPFile) + 1) << 4))
#define G_REMOVE_CASTLE_RIGHT(castle) g_board.gameState &= ~(castle)
#define G_GIVE_CASTLE_RIGHT(castle) g_board.gameState |= castle
#define G_PAWNS(turn) g_board.pieces[0][turn]
#define G_KNIGHTS(turn) g_board.pieces[1][turn]
#define G_BISHOPS(turn) g_board.pieces[2][turn]
#define G_ROOKS(turn) g_board.pieces[3][turn]
#define G_QUEENS(turn) g_board.pieces[4][turn]
#define G_KINGS(turn) g_board.pieces[5][turn]

#define TURN(board) (((board).gameState >> 8) & 1)
#define OPP_TURN(board) (1 - TURN)
#define HAS_CASTLE_RIGHT(board, castle) (((board).gameState & castle) == castle)
#define EP_FILE(board) ((((board).gameState >> 4) & 0x0F) - 1)
#define HALFMOVE_COUNTER(board) ((board).gameState >> 8)
#define SET_EP_FILE(board, EPFile) (board).gameState = (((board).gameState & 0xFF0F) | (((EPFile) + 1) << 4))
#define REMOVE_CASTLE_RIGHT(board, castle) (board).gameState &= ~(castle)
#define GIVE_CASTLE_RIGHT(board, castle) (board).gameState |= castle
#define PAWNS(board, turn) (board).pieces[0][turn]
#define KNIGHTS(board, turn) (board).pieces[1][turn]
#define BISHOPS(board, turn) (board).pieces[2][turn]
#define ROOKS(board, turn) (board).pieces[3][turn]
#define QUEENS(board, turn) (board).pieces[4][turn]
#define KINGS(board, turn) (board).pieces[5][turn]

#define MAX_ARG_COUNT 64
#define MAX_ARG_LENGTH 256

#define TT_TABLE_SIZE 4294967296 //2^32
#define TT_LOOKUP_FAILED -2147483647

#define TT_EXACT 0
#define TT_UPPERBOUND 1
#define TT_LOWERBOUND 2

#define MOVE_INVALID 0

#endif