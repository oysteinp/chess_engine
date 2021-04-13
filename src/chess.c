#include <stdio.h>
#include <sys/time.h>
#include <string.h>

#include "chess.h"

int bestMoveWhite = 0;
int EVALS = 0;
const int SIZE_OF_INT = sizeof(int);
const int BOARD_SIZE = 120;
const int BOARD_MEM_SIZE = BOARD_SIZE * SIZE_OF_INT;

//FEN debuging positions
char start_position[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
char tricky_position[] = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";

//piece encooding
enum pieces {e, P, N, B, R, Q, K, p, n, b, r, q, k, o};

int piece_values[14] = {0,100,300,310,500,900,100000,100,300,310,500,900,100000,0};

enum squares {
    a8=0, b8, c8, d8, e8, f8, g8, h8,
    a7=16, b7, c7, d7, e7, f7, g7, h7,
    a6=32, b6, c6, d6, e6, f6, g6, h6,
    a5=48, b5, c5, d5, e5, f5, g5, h5,
    a4=64, b4, c4, d4, e4, f4, g4, h4,
    a3=80, b3, c3, d3, e3, f3, g3, h3,
    a2=96, b2, c2, d2, e2, f2, g2, h2,
    a1=112, b1, c1, d1, e1, f1, g1, h1, no_sq
};

enum capture_flags  {all_moves, only_captures};

//Castling binary representation
// bin   dec
// 0001    1    white king can castle to the king side
// 0010    2    white king can castle to the queen side 
// 0100    4    black king can castle to the king side 
// 1000    8    black king can castle to the queen side 
//
// Examples
// 1111         Both sides can castle both directions
// 1001         White king => king side
//              Black king => queen side

//Castling rights
enum castling { KC = 1, QC = 2, kc = 4, qc = 8 }; 

//Side to move
enum sides { white, black};

//ascii pieces
char asci_pieces[] = ".PNBRQKpnbrqk"; 

//encode ascii pieces
int char_pieces[] = {
    ['P'] = P,
    ['N'] = N,
    ['B'] = B,
    ['R'] = R,
    ['Q'] = Q,
    ['K'] = K,
    ['p'] = p,
    ['n'] = n,
    ['b'] = b,
    ['r'] = r,
    ['q'] = q,
    ['k'] = k
};

// decode promoted pieces
int promoted_pieces[] = {
    [Q] = 'q',
    [R] = 'r',
    [B] = 'b',
    [N] = 'n',
    [q] = 'q',
    [r] = 'r',
    [b] = 'b',
    [n] = 'n',
};

//unicode pieces
char *unicode_pieces[] = {".", "♟︎", "♞", "♝", "♜", "♛", "♚", "♙", "♘", "♗", "♖", "♕", "♔" };

//Chess board representation
int board[BOARD_SIZE] = {
    r, n, b, q, k, b, n, r,  o, o, o, o, o, o, o, o,
    p, p, p, p, p, p, p, p,  o, o, o, o, o, o, o, o,
    e, e, e, e, e, e, e, e,  o, o, o, o, o, o, o, o,
    e, e, e, e, e, e, e, e,  o, o, o, o, o, o, o, o,
    e, e, e, e, e, e, e, e,  o, o, o, o, o, o, o, o,
    e, e, e, e, e, e, e, e,  o, o, o, o, o, o, o, o,
    P, P, P, P, P, P, P, P,  o, o, o, o, o, o, o, o,
    R, N, B, Q, K, B, N, R
};

int white_pawn_table[BOARD_SIZE] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    50, 50, 50, 50, 50, 50, 50, 50, 0,  0,  0,  0,  0,  0,  0,  0,
    10, 10, 20, 30, 30, 20, 10, 10, 0,  0,  0,  0,  0,  0,  0,  0,
    5,  5, 10, 25, 25, 10,  5,  5,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0, 20, 20,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    5, -5,-10,  0,  0,-10, -5,  5,  0,  0,  0,  0,  0,  0,  0,  0,
    5, 10, 10,-20,-20, 10, 10,  5,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0
};

int black_pawn_table[BOARD_SIZE] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    5, 10, 10,-20,-20, 10, 10,  5,  0,  0,  0,  0,  0,  0,  0,  0,
    5, -5,-10,  0,  0,-10, -5,  5,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0, 20, 20,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    5,  5, 10, 25, 25, 10,  5,  5,  0,  0,  0,  0,  0,  0,  0,  0,
    10, 10, 20, 30, 30, 20, 10, 10, 0,  0,  0,  0,  0,  0,  0,  0,
    50, 50, 50, 50, 50, 50, 50, 50, 0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0
};

int white_knight_table[BOARD_SIZE] = {
    -50,-40,-30,-30,-30,-30,-40,-50, 0,  0,  0,  0,  0,  0,  0,  0,
    -40,-20,  0,  0,  0,  0,-20,-40, 0,  0,  0,  0,  0,  0,  0,  0,
    -30,  0, 10, 15, 15, 10,  0,-30, 0,  0,  0,  0,  0,  0,  0,  0,
    -30,  5, 15, 20, 20, 15,  5,-30, 0,  0,  0,  0,  0,  0,  0,  0,
    -30,  0, 15, 20, 20, 15,  0,-30, 0,  0,  0,  0,  0,  0,  0,  0,
    -30,  5, 10, 15, 15, 10,  5,-30, 0,  0,  0,  0,  0,  0,  0,  0,
    -40,-20,  0,  5,  5,  0,-20,-40, 0,  0,  0,  0,  0,  0,  0,  0,
    -50,-40,-30,-30,-30,-30,-40,-50
};

int black_knight_table[BOARD_SIZE] = {
    -50,-40,-30,-30,-30,-30,-40,-50, 0,  0,  0,  0,  0,  0,  0,  0,
    -40,-20,  0,  5,  5,  0,-20,-40, 0,  0,  0,  0,  0,  0,  0,  0,
    -30,  5, 10, 15, 15, 10,  5,-30, 0,  0,  0,  0,  0,  0,  0,  0,
    -30,  0, 15, 20, 20, 15,  0,-30, 0,  0,  0,  0,  0,  0,  0,  0,
    -30,  5, 15, 20, 20, 15,  5,-30, 0,  0,  0,  0,  0,  0,  0,  0,
    -30,  0, 10, 15, 15, 10,  0,-30, 0,  0,  0,  0,  0,  0,  0,  0,
    -40,-20,  0,  0,  0,  0,-20,-40, 0,  0,  0,  0,  0,  0,  0,  0,
    -50,-40,-30,-30,-30,-30,-40,-50
};

int white_bishop_table[BOARD_SIZE] = {
    -20,-10,-10,-10,-10,-10,-10,-20, 0,  0,  0,  0,  0,  0,  0,  0,
    -10,  0,  0,  0,  0,  0,  0,-10, 0,  0,  0,  0,  0,  0,  0,  0,
    -10,  0,  5, 10, 10,  5,  0,-10, 0,  0,  0,  0,  0,  0,  0,  0,
    -10,  5,  5, 10, 10,  5,  5,-10, 0,  0,  0,  0,  0,  0,  0,  0,
    -10,  0, 10, 10, 10, 10,  0,-10, 0,  0,  0,  0,  0,  0,  0,  0,
    -10, 10, 10, 10, 10, 10, 10,-10, 0,  0,  0,  0,  0,  0,  0,  0,
    -10,  5,  0,  0,  0,  0,  5,-10, 0,  0,  0,  0,  0,  0,  0,  0,
    -20,-10,-10,-10,-10,-10,-10,-20
};

int black_bishop_table[BOARD_SIZE] = {
    -20,-10,-10,-10,-10,-10,-10,-20, 0,  0,  0,  0,  0,  0,  0,  0,
    -10,  5,  0,  0,  0,  0,  5,-10, 0,  0,  0,  0,  0,  0,  0,  0,
    -10, 10, 10, 10, 10, 10, 10,-10, 0,  0,  0,  0,  0,  0,  0,  0,
    -10,  0, 10, 10, 10, 10,  0,-10, 0,  0,  0,  0,  0,  0,  0,  0,
    -10,  5,  5, 10, 10,  5,  5,-10, 0,  0,  0,  0,  0,  0,  0,  0,
    -10,  0,  5, 10, 10,  5,  0,-10, 0,  0,  0,  0,  0,  0,  0,  0,
    -10,  0,  0,  0,  0,  0,  0,-10, 0,  0,  0,  0,  0,  0,  0,  0,
    -20,-10,-10,-10,-10,-10,-10,-20
};

int white_rook_table[BOARD_SIZE] = {
     0,  0,  0,  0,  0,  0,  0,  0, 0,  0,  0,  0,  0,  0,  0,  0,
     5, 10, 10, 10, 10, 10, 10,  5, 0,  0,  0,  0,  0,  0,  0,  0,
    -5,  0,  0,  0,  0,  0,  0, -5, 0,  0,  0,  0,  0,  0,  0,  0,
    -5,  0,  0,  0,  0,  0,  0, -5, 0,  0,  0,  0,  0,  0,  0,  0,
    -5,  0,  0,  0,  0,  0,  0, -5, 0,  0,  0,  0,  0,  0,  0,  0,
    -5,  0,  0,  0,  0,  0,  0, -5, 0,  0,  0,  0,  0,  0,  0,  0,
    -5,  0,  0,  0,  0,  0,  0, -5, 0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  5,  5,  0,  0,  0
};

int black_rook_table[BOARD_SIZE] = {
     0,  0,  0,  5,  5,  0,  0,  0, 0,  0,  0,  0,  0,  0,  0,  0
    -5,  0,  0,  0,  0,  0,  0, -5, 0,  0,  0,  0,  0,  0,  0,  0, 
    -5,  0,  0,  0,  0,  0,  0, -5, 0,  0,  0,  0,  0,  0,  0,  0,
    -5,  0,  0,  0,  0,  0,  0, -5, 0,  0,  0,  0,  0,  0,  0,  0,
    -5,  0,  0,  0,  0,  0,  0, -5, 0,  0,  0,  0,  0,  0,  0,  0,
    -5,  0,  0,  0,  0,  0,  0, -5, 0,  0,  0,  0,  0,  0,  0,  0,
     5, 10, 10, 10, 10, 10, 10,  5, 0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0
};

int white_queen_table[BOARD_SIZE] = {
    -20,-10,-10, -5, -5,-10,-10,-20,0,  0,  0,  0,  0,  0,  0,  0,
    -10,  0,  0,  0,  0,  0,  0,-10,0,  0,  0,  0,  0,  0,  0,  0,
    -10,  0,  5,  5,  5,  5,  0,-10,0,  0,  0,  0,  0,  0,  0,  0,
     -5,  0,  5,  5,  5,  5,  0, -5,0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  5,  5,  5,  5,  0, -5,0,  0,  0,  0,  0,  0,  0,  0,
    -10,  5,  5,  5,  5,  5,  0,-10,0,  0,  0,  0,  0,  0,  0,  0,
    -10,  0,  5,  0,  0,  0,  0,-10,0,  0,  0,  0,  0,  0,  0,  0,
    -20,-10,-10, -5, -5,-10,-10,-20
};

int black_queen_table[BOARD_SIZE] = {
    -20,-10,-10, -5, -5,-10,-10,-20, 0,  0,  0,  0,  0,  0,  0,  0,
    -10,  0,  5,  0,  0,  0,  0,-10, 0,  0,  0,  0,  0,  0,  0,  0,
    -10,  5,  5,  5,  5,  5,  0,-10, 0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  5,  5,  5,  5,  0, -5, 0,  0,  0,  0,  0,  0,  0,  0,
     -5,  0,  5,  5,  5,  5,  0, -5, 0,  0,  0,  0,  0,  0,  0,  0,
    -10,  0,  5,  5,  5,  5,  0,-10, 0,  0,  0,  0,  0,  0,  0,  0,
    -10,  0,  0,  0,  0,  0,  0,-10, 0,  0,  0,  0,  0,  0,  0,  0,
    -20,-10,-10, -5, -5,-10,-10,-20
};

int white_king_table[BOARD_SIZE] = {
    -30,-40,-40,-50,-50,-40,-40,-30, 0,  0,  0,  0,  0,  0,  0,
    -30,-40,-40,-50,-50,-40,-40,-30, 0,  0,  0,  0,  0,  0,  0,
    -30,-40,-40,-50,-50,-40,-40,-30, 0,  0,  0,  0,  0,  0,  0,
    -30,-40,-40,-50,-50,-40,-40,-30, 0,  0,  0,  0,  0,  0,  0,
    -20,-30,-30,-40,-40,-30,-30,-20, 0,  0,  0,  0,  0,  0,  0,
    -10,-20,-20,-20,-20,-20,-20,-10, 0,  0,  0,  0,  0,  0,  0,
    20, 20,  0,  0,  0,  0, 20, 20,  0,  0,  0,  0,  0,  0,  0,
    20, 30, 10,  0,  0, 10, 30, 20
};

int black_king_table[BOARD_SIZE] = {
     20, 30, 10,  0,  0, 10, 30, 20,  0,  0,  0,  0,  0,  0,  0,
     20, 20,  0,  0,  0,  0, 20, 20,  0,  0,  0,  0,  0,  0,  0,
    -10,-20,-20,-20,-20,-20,-20,-10,  0,  0,  0,  0,  0,  0,  0,
    -20,-30,-30,-40,-40,-30,-30,-20,  0,  0,  0,  0,  0,  0,  0,
    -30,-40,-40,-50,-50,-40,-40,-30,  0,  0,  0,  0,  0,  0,  0,
    -30,-40,-40,-50,-50,-40,-40,-30,  0,  0,  0,  0,  0,  0,  0,
    -30,-40,-40,-50,-50,-40,-40,-30,  0,  0,  0,  0,  0,  0,  0,
    -30,-40,-40,-50,-50,-40,-40,-30
};

//Side to move
int side = white;

//enpassant square
int enpassant = no_sq;

//Castling rights
int castle = 15; //dec 15 => bin 1111 => all castling available

int king_squares[2] = {e1,e8};

/*
    Move formatting
    
    0000 0000 0000 0000 0111 1111       source square
    0000 0000 0011 1111 1000 0000       target square
    0000 0011 1100 0000 0000 0000       promoted piece
    0000 0100 0000 0000 0000 0000       capture flag
    0000 1000 0000 0000 0000 0000       double pawn flag
    0001 0000 0000 0000 0000 0000       enpassant flag
    0010 0000 0000 0000 0000 0000       castling
*/

// encode move
#define encode_move(source, target, promoted_piece, capture, double_pawn_push, enpass, castling) \
(                          \
    (source) |             \
    (target << 7) |        \
    (promoted_piece << 14) |        \
    (capture << 18) |      \
    (double_pawn_push << 19) |         \
    (enpass << 20) |    \
    (castling << 21)       \
)

// decode move's source square
#define get_move_source(move) (move & 0x7f)

// decode move's target square
#define get_move_target(move) ((move >> 7) & 0x7f)

// decode move's promoted piece
#define get_move_piece(move) ((move >> 14) & 0xf)

// decode move's capture flag
#define get_move_capture(move) ((move >> 18) & 0x1)

// decode move's double pawn push flag
#define get_move_pawn(move) ((move >> 19) & 0x1)

// decode move's enpassant flag
#define get_move_enpassant(move) ((move >> 20) & 0x1)

// decode move's castling flag
#define get_move_castling(move) ((move >> 21) & 0x1)

//Convert board square indexes to coordinates
char *square_to_coords[] = {
    "a8","b8","c8","d8","e8","f8","g8","h8","i8","j8","k8","l8","m8","n8","o8", "p8",
    "a7","b7","c7","d7","e7","f7","g7","h7","i7","j7","k7","l7","m7","n7","o7", "p7",
    "a6","b6","c6","d6","e6","f6","g6","h6","i6","j6","k6","l6","m6","n6","o6", "p6",
    "a5","b5","c5","d5","e5","f5","g5","h5","i5","j5","k5","l5","m5","n5","o5", "p5",
    "a4","b4","c4","d4","e4","f4","g4","h4","i4","j4","k4","l4","m4","n4","o4", "p4",
    "a3","b3","c3","d3","e3","f3","g3","h3","i3","j3","k3","l3","m3","n3","o3", "p3",
    "a2","b2","c2","d2","e2","f2","g2","h2","i2","j2","k2","l2","m2","n2","o2", "p2",
    "a1","b1","c1","d1","e1","f1","g1","h1","i1","j1","k1","l1","m1","n1","o1", "p1"
};

//Piece move offsets
int knight_offsets[8] = {33,31,18,14,-14,-18,-31,-33};
int bishop_offsets[4] = {15,17,-15,-17};
int rook_offsets[4] = {16,1,-1,-16};
int king_offsets[8] = {15,17,-15,-17,16,1,-1,-16};    

//Move list structure
typedef struct {
    //Move list
    int moves[256];
    //Move count;
    int count;

    //Captures
    int captures[256];
    int captureCount;

    int noCaptures[256];
    int noCaptureCount;
} moves;

void print_board() {

    printf("\n");

    for(int rank = 0; rank < 8; rank++) {
        for(int file = 0; file < 16; file++) {
            //init square
            int square = rank * 16 + file;

            if(file == 0) {
                printf(" %d ", 8 - rank);
            }
            
            if(!(square & 0x88)) {
                //printf("%c ", asci_pieces[board[square]]);
                printf("%s ", unicode_pieces[board[square]]);
            }
        }
        printf("\n");
    }

    printf("\n   a b c d e f g h \n\n");

    printf("   Board stats\n");
    printf("   Side to move:    %s\n", (side == white) ? "white" : "black");
    printf("   Castling:        %c%c%c%c\n", (castle & KC) ? 'K':'-', 
                                    (castle & QC) ? 'Q':'-',
                                    (castle & kc) ? 'k':'-', 
                                    (castle & qc) ? 'q':'-' );
    printf("   Enpassant:       %s\n", (enpassant == no_sq) ? "n/a" : square_to_coords[enpassant]);
    printf("   King's square:   %s\n\n", square_to_coords[king_squares[side]]);
    printf("\n");
}

// print move list
void print_move_list(moves *move_list)
{
    // print table header
    printf("\n    Move     Capture  Double   Enpass   Castling\n\n");

    // loop over moves in a movelist
    for (int index = 0; index < move_list->count; index++)
    {
        int move = move_list->moves[index];
        printf("    %s%s", square_to_coords[get_move_source(move)], square_to_coords[get_move_target(move)]);
        printf("%c    ", get_move_piece(move) ? promoted_pieces[get_move_piece(move)] : ' ');
        printf("%d        %d        %d        %d\n", get_move_capture(move), get_move_pawn(move), get_move_enpassant(move), get_move_castling(move));
    }
    
    printf("\n    Total moves: %d\n\n", move_list->count);
}

int is_attacked_by_sliding_piece(int square, int side, int offsets[], int white_piece, int black_piece) {
    for(int i = 0; i < 4; i++) {
        int target_square = square + offsets[i];

        while(!((target_square) & 0x88)) {
            int target_piece = board[target_square];
            if(side == white ? (target_piece == white_piece || target_piece == Q) : (target_piece == black_piece || target_piece == q)) {
                return 1;
            }
            if(target_piece) {
                break;
            }
            target_square += offsets[i];
        }
    }
    return 0;
}

int is_attacked_by_jumping_piece(int square, int side, int offsets[], int white_piece, int black_piece) {
    for(int i = 0; i < 8; i++) {
        int target_square = square + offsets[i];

        if(!(target_square & 0x88)) {
            int target_piece = board[target_square];
            if(side == white ? target_piece == white_piece : target_piece == black_piece) {
                return 1;
            }
        }
    }
    return 0;
}

int is_square_attacked(int square, int side) {
    //pawn attacks
    if(side == white) {
        if(!((square + 17) & 0x88) && (board[square + 17] == P)) {
            return 1;
        }
        if(!((square + 15) & 0x88) && (board[square + 15] == P)) {
            return 1;
        }
    }
    else {
        if(!((square - 17) & 0x88) && (board[square - 17] == p)) {
            return 1;
        }
        if(!((square - 15) & 0x88) && (board[square - 15] == p)) {
            return 1;
        }
    }
    
    if(is_attacked_by_jumping_piece(square, side, knight_offsets, N, n)) {
        return 1;
    }

    if(is_attacked_by_jumping_piece(square, side, king_offsets, K, k)) {
        return 1;
    }

    if(is_attacked_by_sliding_piece(square, side, rook_offsets, R, r)) {
        return 1;
    }
    if(is_attacked_by_sliding_piece(square, side, bishop_offsets, B, b)) {
        return 1;
    }

    return 0;
}

void print_attached_squares(int side) {
    printf("\n");
    printf("    Attacking side: %s\n\n", (side==white) ? "white" : "black");

    for(int rank = 0; rank < 8; rank++) {
        for(int file = 0; file < 16; file++) {
            //init square
            int square = rank * 16 + file;

            if(file == 0) {
                printf(" %d ", 8 - rank);
            }
            
            if(!(square & 0x88)) {
                printf("%c ", is_square_attacked(square, side) ? 'x' : '.');
            }

            
        }
        printf("\n");
    }
    printf("\n   a b c d e f g h\n");
}

void reset_board() {
    for(int rank = 0; rank < 8; rank++) {
        for(int file = 0; file < 16; file++) {
            int square = rank * 16 + file;

            if(!(square & 0x88)) {
                board[square] = e;
            }
        }
    }

    //Reset stats
    side = -1;
    castle = 0;
    enpassant = no_sq;
}

void parse_fen(char *fen) {
    reset_board();

    for(int rank = 0; rank < 8; rank++) {
        for(int file = 0; file < 16; file++) {
            int square = rank * 16 + file;

            if(!(square & 0x88)) {
                if((*fen >= 'a' && *fen<='z') || (*fen >= 'A' && *fen<='Z')) {
                    if(*fen == 'K') {
                        king_squares[white] = square;
                    } else if(*fen == 'k') {
                        king_squares[black] = square;
                    }
                    board[square] = char_pieces[*fen];
                    *fen++;
                }
                //Match empty squares
                if(*fen >= '0' && *fen <= '8') {
                    int offset = *fen - '0'; //character to integer

                    //Decrement file om empty squares
                    if(!board[square]) {
                        file--;
                    }
                    
                    //Skip empty squares
                    file += offset;

                    *fen++;
                }
                //Match end of rank
                if(*fen == '/') {
                    *fen++;
                }
               // printf("square: %s | current FEN char: %c \n", square_to_coords[square], *fen);
                
            }
        }
    }

    //Go to side parsing
    *fen++;

    //Parse side to move
    side = (*fen == 'w') ? white : black;

    //Go to castling parsing
    fen += 2;

    //Parse castling rights
    while(*fen != ' ') {
        switch(*fen) {
            case 'K': castle |= KC; break;
            case 'Q': castle |= QC; break; 
            case 'k': castle |= kc; break;
            case 'q': castle |= qc; break;
            case '-': break;
        }
        *fen++; 
    }

    //Go to enpassant square
    *fen++;

    //Parse enpassant square
    if(*fen != '-') {
        //Parse enpassant square's rank and file
        int file = fen[0] - 'a';
        int rank = 8 - (fen[1] - '0'); 
        
        //Set enpassant square
        enpassant = rank*16 + file;  

    } else {
        enpassant = no_sq;
    }
}

void add_move(moves *move_list, int move) {
    if(get_move_capture(move)) {
        move_list->captures[move_list->captureCount] = move;
        move_list->captureCount++;
    } else {
        move_list->noCaptures[move_list->noCaptureCount] = move;
        move_list->noCaptureCount++;
    }
}

void generate_moves_for_jumping_piece(moves *move_list, int square, int side, int offsets[], int size, int white_piece, int black_piece) {
    if (side == white ? board[square] == white_piece : board[square] == black_piece) {
        for(int index = 0; index < size; index++) {
            int target_square = square + offsets[index];

            if(!(target_square & 0x88)) {
                if(side == white) {
                    if(board[target_square] == e) {
                        add_move(move_list, encode_move(square, target_square, 0, 0, 0, 0 ,0));
                    } else if(board[target_square] >= p && board[target_square] <= k) {
                        add_move(move_list, encode_move(square, target_square, 0, 1, 0, 0 ,0));
                    }
                } else {
                    if(board[target_square] == e) {
                        add_move(move_list, encode_move(square, target_square, 0, 0, 0, 0 ,0));
                    } else if(board[target_square] >= P && board[target_square] <= K) {
                        add_move(move_list, encode_move(square, target_square, 0, 1, 0, 0 ,0));
                    }
                }
            }
        }
    }
}

void generate_moves_for_sliding_piece(moves *move_list, int square, int side, int offsets[], int size, int white_piece, int black_piece) {
    int target_piece = 0;
    if (side == white ? (board[square] == white_piece || board[square] == Q) : (board[square] == black_piece || board[square] == q)) {
        for(int index = 0; index < size; index++) {
            int target_square = square + offsets[index];
            
            while(!((target_square) & 0x88)) {
                target_piece = board[target_square];
                if(target_piece == e) {
                    add_move(move_list, encode_move(square, target_square, 0, 0, 0, 0 ,0));
                } else if(side == white && target_piece >= p && target_piece <= k) {
                    add_move(move_list, encode_move(square, target_square, 0, 1, 0, 0 ,0));
                    break;
                } else if(side == black && target_piece >= P && target_piece <= K) {
                    add_move(move_list, encode_move(square, target_square, 0, 1, 0, 0 ,0));
                    break;
                } else {
                    break;
                }
                target_square += offsets[index];
            }
        }
    }
}

void generate_castling_moves(moves *move_list, int side) {

    if(side == white && board[e1] == K) {
        //Is white king side castling available?
        if(castle & KC && board[f1] == e && board[g1] == e) {
            //Make sure king and next square is not under attack
            if(!is_square_attacked(e1, black) && !is_square_attacked(f1, black)) {
                add_move(move_list, encode_move(e1, g1, 0, 0, 0, 0 ,1));
            }
        }

        //Is white queen side castling available?
        if(castle & QC && board[b1] == e && board[c1] == e && board[d1] == e) {
            //Make sure king and next square is not under attack
            if(!is_square_attacked(e1, black) && !is_square_attacked(d1, black)) {
                add_move(move_list, encode_move(e1, c1, 0, 0, 0, 0 ,1));
            }
        }
    } else if(side == black && board[e8] == k) {
        //Is black king side castling available?
        if(castle & kc && board[f8] == e && board[g8] == e) {
            //Make sure king and next square is not under attack
            if(!is_square_attacked(e8, white) && !is_square_attacked(f8, white)) {
                add_move(move_list, encode_move(e8, g8, 0, 0, 0, 0 ,1));
            }
        }

        //Is black queen side castling available?
        if(castle & qc && board[b8] == e && board[c8] == e && board[d8] == e) {
            //Make sure king and next square is not under attack
            if(!is_square_attacked(e8, white) && !is_square_attacked(d8, white)) {
                add_move(move_list, encode_move(e8, c8, 0, 0, 0, 0 ,1));
            }
        }
    }
}

void generate_moves(moves *move_list) {

    move_list->count = 0;
    move_list->captureCount = 0;
    move_list->noCaptureCount = 0;

    for(int square = 0; square < 120; square ++) {
        if(!(square & 0x88)) {

            //White pawn and castling moves
            if(side == white) {
                if(board[square] == P) {
                    //quiet
                    int to_square = square - 16;

                    //check if target square is on board
                    if(board[to_square] == e) {
                        //pawn promotions
                        if(square >= a7 && square <= h7) {
                            add_move(move_list, encode_move(square, to_square, Q, 0, 0, 0 ,0));
                            add_move(move_list, encode_move(square, to_square, R, 0, 0, 0 ,0));
                            add_move(move_list, encode_move(square, to_square, B, 0, 0, 0 ,0));
                            add_move(move_list, encode_move(square, to_square, N, 0, 0, 0 ,0));
                        } else {
                            //one square ahead
                            add_move(move_list, encode_move(square, to_square, 0, 0, 0, 0 ,0));

                            //two squares ahead
                            to_square -= 16;
                            if(square >= a2 && square <= h2 && board[to_square] == e) {
                                add_move(move_list, encode_move(square, to_square, 0, 0, 1, 0 ,0));
                            }
                        }
                    }

                    //White pawn captures
                    to_square = square - 15;
                    int enpassant_move = (to_square == enpassant);
                    if(!(to_square & 0x88) && ((board[to_square] >= p && board[to_square] <= q) || enpassant_move)) {
                        if(square >= a7 && square <= h7) {
                            add_move(move_list, encode_move(square, to_square, Q, 1, 0, 0 ,0));
                            add_move(move_list, encode_move(square, to_square, R, 1, 0, 0 ,0));
                            add_move(move_list, encode_move(square, to_square, B, 1, 0, 0 ,0));
                            add_move(move_list, encode_move(square, to_square, N, 1, 0, 0 ,0));
                        } else {
                            add_move(move_list, encode_move(square, to_square, 0, 1, 0, enpassant_move, 0));
                        }
                    }
                    to_square = square - 17;
                    enpassant_move = (to_square == enpassant);
                    if(!(to_square & 0x88) && ((board[to_square] >= p && board[to_square] <= q) || enpassant_move)) {
                        if(square >= a7 && square <= h7) {
                            add_move(move_list, encode_move(square, to_square, Q, 1, 0, 0 ,0));
                            add_move(move_list, encode_move(square, to_square, R, 1, 0, 0 ,0));
                            add_move(move_list, encode_move(square, to_square, B, 1, 0, 0 ,0));
                            add_move(move_list, encode_move(square, to_square, N, 1, 0, 0 ,0));
                        } else {
                            add_move(move_list, encode_move(square, to_square, 0, 1, 0, enpassant_move, 0));
                        }
                    }
                }
            } else {
                if(board[square] == p) {
                    int to_square = square + 16;

                    //check if target square is on board
                    if(board[to_square] == e) {
                        
                        //pawn promotions
                        if(square >= a2 && square <= h2) {
                            add_move(move_list, encode_move(square, to_square, q, 0, 0, 0 ,0));
                            add_move(move_list, encode_move(square, to_square, r, 0, 0, 0 ,0));
                            add_move(move_list, encode_move(square, to_square, b, 0, 0, 0 ,0));
                            add_move(move_list, encode_move(square, to_square, n, 0, 0, 0 ,0));
                        } else {
                            //one square ahead
                            add_move(move_list, encode_move(square, to_square, 0, 0, 0, 0 ,0));

                            //two squares ahead
                            to_square += 16;
                            if(square >= a7 && square <= h7 && board[to_square] == e) {
                                add_move(move_list, encode_move(square, to_square, 0, 0, 1, 0 ,0));
                            }
                        }
                    }

                    //Black pawn captures
                    to_square = square + 15;
                    int enpassant_move = (to_square == enpassant);
                    if(!(to_square & 0x88) && ((board[to_square] >= P && board[to_square] <= Q) || enpassant_move)) {
                        if(square >= a2 && square <= h2) {
                            add_move(move_list, encode_move(square, to_square, q, 1, 0, 0 ,0));
                            add_move(move_list, encode_move(square, to_square, r, 1, 0, 0 ,0));
                            add_move(move_list, encode_move(square, to_square, b, 1, 0, 0 ,0));
                            add_move(move_list, encode_move(square, to_square, n, 1, 0, 0 ,0));
                        } else {
                            add_move(move_list, encode_move(square, to_square, 0, 1, 0, enpassant_move ,0));
                        }
                    }
                    to_square = square + 17;
                    enpassant_move = (to_square == enpassant);
                    if(!(to_square & 0x88) && ((board[to_square] >= P && board[to_square] <= Q) || enpassant_move)) {
                        if(square >= a2 && square <= h2) {
                            add_move(move_list, encode_move(square, to_square, q, 1, 0, 0 ,0));
                            add_move(move_list, encode_move(square, to_square, r, 1, 0, 0 ,0));
                            add_move(move_list, encode_move(square, to_square, b, 1, 0, 0 ,0));
                            add_move(move_list, encode_move(square, to_square, n, 1, 0, 0 ,0));
                        } else {
                            add_move(move_list, encode_move(square, to_square, 0, 1, 0, enpassant_move ,0));
                        }
                    }
                }
            }
            generate_moves_for_jumping_piece(move_list, square, side, knight_offsets, 8, N, n);
            generate_moves_for_jumping_piece(move_list, square, side, king_offsets, 8, K, k);
            generate_moves_for_sliding_piece(move_list, square, side, bishop_offsets, 4, B, b);
            generate_moves_for_sliding_piece(move_list, square, side, rook_offsets, 4, R, r);
        }
    }

    generate_castling_moves(move_list, side);

    //Add moves to same list
    int size = move_list->captureCount;
    for(int move_count = 0; move_count < size; move_count++) {
        int move = move_list->captures[move_count];
        move_list->moves[move_list->count] = move;
        move_list->count++;
    }

    size = move_list->noCaptureCount;
    for(int move_count = 0; move_count < size; move_count++) {
        int move = move_list->noCaptures[move_count];
        move_list->moves[move_list->count] = move;
        move_list->count++;
    }
}

int make_move(int move, int capture_flag) {
    EVALS++;
    if(capture_flag == all_moves) {

        // Define board state variable copies ((should be in "class" or struct??)
        int board_copy[BOARD_SIZE];
        int side_copy;
        int enpassant_copy;
        int castle_copy;
        int king_squares_copy[2];

        //Copy
        memcpy(board_copy, board, BOARD_MEM_SIZE);
        memcpy(king_squares_copy, king_squares, 8);
        side_copy = side;
        enpassant_copy = enpassant;
        castle_copy = castle;

        //Parse move
        int from_square = get_move_source(move);
        int to_square = get_move_target(move);
        int promoted_piece = get_move_piece(move);
        int enpass = get_move_enpassant(move);
        int double_p_push = get_move_pawn(move);
        int castling = get_move_castling(move);
        
        //Make move
        board[to_square] = board[from_square];
        board[from_square] = e;
        if(promoted_piece) {
            board[to_square] = promoted_piece;
        }
        if(enpass) {
            if(side == white) {
                board[to_square+16] = e;
            } else {
                board[to_square-16] = e;    
            }
        }
        if(double_p_push) {
            if(side == white) {
                enpassant = to_square+16;
            } else {
                enpassant = to_square-16;   
            }
        } else {
            enpassant = no_sq;
        }
        if(castling) {
            if(to_square == g1) {
                board[f1] = board[h1];
                board[h1] = e;
            } else if(to_square == c1) {
                board[d1] = board[a1];
                board[a1] = e;
            } else if(to_square == g8) {
                board[f8] = board[h8];
                board[h8] = e;
            } else if(to_square == c8) {
                board[d8] = board[a8];
                board[a8] = e;
            }
        }

        if(board[to_square] == K || board[to_square] == k) {
            king_squares[side] = to_square;
        }
        
        //Update castling rights
        if(board[to_square] == K) {
            castle = castle & 0xe;
            castle = castle & 0xd;
        } else if(board[to_square] == k) {
            castle = castle & 0xb;
            castle = castle & 0x7;
        }
        if(to_square == h1 || from_square == h1) {
            castle = castle & 0xe;
        } if(to_square == a1 || from_square == a1) {
            castle = castle & 0xd;
        } if(to_square == h8 || from_square == h8) {
            castle = castle & 0xb;
        } if(to_square == a8 || from_square == a8) {
            castle = castle & 0x7;
        }

        //Check if king is in check after move
        //king_squares[side]
        if(is_square_attacked(king_squares[side], side ^ 1)) {
            //Restore board
            memcpy(board, board_copy, BOARD_MEM_SIZE);
            memcpy(king_squares, king_squares_copy, 8);
            side = side_copy;
            enpassant = enpassant_copy;
            castle = castle_copy;
            return 0;
        }

        //Change side
        side ^= 1;

        return 1;
    }
    // capture move
    else if (get_move_capture(move)) {
        return make_move(move, all_moves);
    }
    return 0;
}

//Count nodes
long nodes = 0;

int getTimeInMs() {
    struct timeval t;
    gettimeofday(&t, NULL);
    return t.tv_sec * 1000 + t.tv_usec / 1000;
}

void perft(int depth) {

    //escape condition
    if(depth == 0) {
        nodes++;
        return;
    }

    moves move_list[1];

    generate_moves(move_list);

    //Loop over generated moves
    int size = move_list->count;
    for(int move_count = 0; move_count < size; move_count++) {
         // Define board state variable copies ((should be in "class" or struct??)
        int board_copy[BOARD_SIZE];
        int side_copy;
        int enpassant_copy;
        int castle_copy;
        int king_squares_copy[2];

        //Copy
        memcpy(board_copy, board, BOARD_MEM_SIZE);
        memcpy(king_squares_copy, king_squares, 8);
        side_copy = side;
        enpassant_copy = enpassant;
        castle_copy = castle;

        if(!make_move(move_list->moves[move_count], all_moves)) {
            continue;
        }

        //Make recursive call
        perft(depth-1);

         //Restore board
        memcpy(board, board_copy, BOARD_MEM_SIZE);
        memcpy(king_squares, king_squares_copy, 8);
        side = side_copy;
        enpassant = enpassant_copy;
        castle = castle_copy;
    }
}

int perft_test(int depth, int debug) {

    //Init start time
    int start_time = getTimeInMs();

    if(debug)
        printf("\nPerformance test\n\n");
    moves move_list[1];

    generate_moves(move_list);

    //Loop over generated moves
    int size = move_list->count;
    for(int move_count = 0; move_count < size; move_count++) {
         // Define board state variable copies ((should be in "class" or struct??)
        int board_copy[BOARD_SIZE];
        int side_copy;
        int enpassant_copy;
        int castle_copy;
        int king_squares_copy[2];

        //Copy
        memcpy(board_copy, board, BOARD_MEM_SIZE);
        memcpy(king_squares_copy, king_squares, 8);
        side_copy = side;
        enpassant_copy = enpassant;
        castle_copy = castle;

        if(!make_move(move_list->moves[move_count], all_moves)) {
            continue;
        }

        //Cummulative nodes
        long total_nodes = nodes;

        //Make recursive call
        perft(depth-1);

        //Old nodes
        long old_nodes = nodes - total_nodes;

         //Restore board
        memcpy(board, board_copy, BOARD_MEM_SIZE);
        memcpy(king_squares, king_squares_copy, 8);
        side = side_copy;
        enpassant = enpassant_copy;
        castle = castle_copy;

        //Print move
        int currentMove = move_list->moves[move_count];
        if(debug) {
            printf(" move  %d: %s%s%c     %ld\n",
            move_count+1, 
            square_to_coords[get_move_source(currentMove)],
            square_to_coords[get_move_target(currentMove)],
            promoted_pieces[get_move_piece(currentMove)],
            old_nodes);
        }
        
    }
    int time = getTimeInMs() - start_time;
    printf("\n Depth: %d", depth);
    printf("\n Nodes: %ld", nodes);
    printf("\n Time: %d ms", time);
    double secs = time/1000.0;
    printf("\n Nodes/sec: %.0f\n", time>0?nodes/secs:0);
    
    return nodes;
}

int perft_test2(int depth, char *fen) {
    parse_fen(fen);
    nodes = 0;
    int nodes = perft_test(depth, 0);
    return nodes;
}

void test(int expected, int actual, const char* testname) {
    if(expected == actual) {
        printf("%s PASSED\n", testname);
    } else {
        printf("\n%s FAILED expected: %d actual: %d", testname, expected, actual);
    }
}

int count_material (int board[]) {
    int result = 0;
    for(int square = 0; square < 120; square ++) {
        if(!(square & 0x88)) {
            int current_piece = board[square];
            if(current_piece >= P && current_piece <= Q) {
                result += piece_values[current_piece];
            } else if (current_piece >= p && current_piece <= q){
                result -= piece_values[current_piece];
            }
        }
    }
    return result;
}

int forceKingToCorner(int movingKingSquare, int opponentKingSquare, int opponentNumberOfPieces) {
    if(opponentNumberOfPieces > 4) {
        return 0;
    }
    int opponentKingRank = 8-(opponentKingSquare/16);
    int opponentKingFile = 1 + (opponentKingSquare % 16);

    int oppoonentKingDistanceToCentreRank = 0;
    if(3-opponentKingRank > opponentKingRank -4) {
        oppoonentKingDistanceToCentreRank = 3-opponentKingRank;
    } else {
        oppoonentKingDistanceToCentreRank = opponentKingRank -4;
    }

    int oppoonentKingDistanceToCentreFile = 0;
    if(3-opponentKingFile > opponentKingFile -4) {
        oppoonentKingDistanceToCentreFile = 3-opponentKingFile;
    } else {
        oppoonentKingDistanceToCentreFile = opponentKingFile -4;
    }
    int opponentKingDistanceFromCentre = oppoonentKingDistanceToCentreRank + oppoonentKingDistanceToCentreFile;

    int movingKingRank = 8-(movingKingSquare/16);
    int movingKingFile = 1 + (movingKingSquare % 16);

    int distBetweenKingsFile = (movingKingFile - opponentKingFile > 0) ? (movingKingFile - opponentKingFile > 0) : (opponentKingFile - movingKingFile);
    int distBetweenKingsRank = (movingKingRank - opponentKingRank > 0) ? (movingKingRank - opponentKingRank > 0) : (opponentKingRank - movingKingRank);
    int distBetweenKings = distBetweenKingsFile + distBetweenKingsRank;

    opponentKingDistanceFromCentre += (14-distBetweenKings);

    return opponentKingDistanceFromCentre;
}

int evaluatePiecePositions() {
    int whiteScore = 0;
    int blackScore = 0;

    for(int i = 0; i<BOARD_SIZE; i++) {
        int square = board[i];
        if(square == e) {
            continue;
        }
        if(!(square & 0x88)) {
            if(square == P) {
                whiteScore += white_pawn_table[i];
            } else if(square == p) {
                blackScore += black_pawn_table[i];
            } else if(square == N) {
                whiteScore += white_knight_table[i];
            } else if(square == n) {
                blackScore += black_king_table[i];
            } else if(square == B) {
                whiteScore += white_bishop_table[i];
            } else if(square == b) {
                blackScore += black_bishop_table[i];
            } else if(square == R) {
                whiteScore += white_rook_table[i];
            } else if(square == r) {
                blackScore += black_rook_table[i];
            } else if(square == Q) {
                whiteScore += white_queen_table[i];
            } else if(square == q) {
                blackScore += black_queen_table[i];
            } else if(square == K) {
                whiteScore += white_king_table[i];
            } else if(square == k) {
                blackScore += black_king_table[i];
            }
        }
    }
    if(side == white) {
        return whiteScore-blackScore;
    } 
    return blackScore-whiteScore;
}

typedef struct LINE {
    int cmove;              // Number of moves in the line.
    int argmove[10];  // The line.
}   LINE;


int evaluate(int board[]) {
    int material = count_material(board);

    int piecePositions = evaluatePiecePositions();
    //int endgameEval = 0; //forceKingToCorner(king_squares[side], king_squares[!side], 1);

    return  material + piecePositions;
}

void print_move(int move, int newLine) {
    int isMoveCapture = get_move_capture(move);
    char* capture = "";
    if(isMoveCapture) {
        capture = "x";
    }
    if(newLine) {
        printf("%s%s%s\n", square_to_coords[get_move_source(move)], capture, square_to_coords[get_move_target(move)]);
    } else {
        printf("%s%s%s", square_to_coords[get_move_source(move)], capture, square_to_coords[get_move_target(move)]);
    }
}

int areThereAnyLegalMoves() {
    moves move_list[1];
    generate_moves(move_list);
    int size = move_list->count;
    for(int move_count = 0; move_count < size; move_count++) {
        //Make move
        int board_copy[BOARD_SIZE];
        int side_copy;
        int enpassant_copy;
        int castle_copy;
        int king_squares_copy[2];

        //Copy
        memcpy(board_copy, board, BOARD_MEM_SIZE);
        memcpy(king_squares_copy, king_squares, 8);
        side_copy = side;
        enpassant_copy = enpassant;
        castle_copy = castle;

        int move = move_list->moves[move_count];
        if(!make_move(move, all_moves)) {
            continue;
        }

        //Restore board
        memcpy(board, board_copy, BOARD_MEM_SIZE);
        memcpy(king_squares, king_squares_copy, 8);
        side = side_copy;
        enpassant = enpassant_copy;
        castle = castle_copy;

        return 1;
    }
    return 0;
}

int QuiescenceSearch (int alpha, int beta) {
    int eval = evaluate(board);
    if (eval >= beta) {
        return beta;
    }
    if (eval > alpha) {
        alpha = eval;
    }

    moves move_list[1];
    generate_moves(move_list);
    int size = move_list->count;

    for (int i = 0; i < size; i++) {
        //Make move
        int board_copy[BOARD_SIZE];
        int side_copy;
        int enpassant_copy;
        int castle_copy;
        int king_squares_copy[2];

        //Copy
        memcpy(board_copy, board, BOARD_MEM_SIZE);
        memcpy(king_squares_copy, king_squares, 8);
        side_copy = side;
        enpassant_copy = enpassant;
        castle_copy = castle;

        int move = move_list->moves[size];
        
        if(!make_move(move, only_captures)) {
            continue;
        }
        eval = -QuiescenceSearch (-beta, -alpha);
        //Restore board
        memcpy(board, board_copy, BOARD_MEM_SIZE);
        memcpy(king_squares, king_squares_copy, 8);
        side = side_copy;
        enpassant = enpassant_copy;
        castle = castle_copy;

        if (eval >= beta) {
            return beta;
        }
        if (eval > alpha) {
            alpha = eval;
        }
    }

			return alpha;
		}

int minimax(int initialDepth, int depth, int maximizingPlayer, int alpha, int beta, LINE * pline) {
    LINE line;
    line.cmove = 0;
    if(depth == 0) {
        pline->cmove = 0;
        //int score = evaluate(board);
        int score = QuiescenceSearch (alpha, beta);
        return score;
    } 
    moves move_list[1];
    generate_moves(move_list);
    int size = move_list->count;

    if(maximizingPlayer) {
        int maxEval = -9999999;

        //Loop over generated moves
        int foundLegalMove = 0;
        for(int move_count = 0; move_count < size; move_count++) {
            //Make move
            int board_copy[BOARD_SIZE];
            int side_copy;
            int enpassant_copy;
            int castle_copy;
            int king_squares_copy[2];

            //Copy
            memcpy(board_copy, board, BOARD_MEM_SIZE);
            memcpy(king_squares_copy, king_squares, 8);
            side_copy = side;
            enpassant_copy = enpassant;
            castle_copy = castle;

            int move = move_list->moves[move_count];
            
            if(!make_move(move, all_moves)) {
                continue;
            } 
            foundLegalMove = 1;
            int eval = minimax(initialDepth, depth -1, 0, alpha, beta, &line);
            if(eval > maxEval) {
                if(depth == initialDepth) {
                    bestMoveWhite = move;
                }
                maxEval = eval;
            } 
            if(eval > alpha) {
                pline->argmove[0] = move;
                memcpy(pline->argmove + 1, line.argmove, line.cmove * SIZE_OF_INT);
                pline->cmove = line.cmove + 1;
                alpha = eval;
            }
            if(beta <= alpha) {
                break;
            }

            //Restore board
            memcpy(board, board_copy, BOARD_MEM_SIZE);
            memcpy(king_squares, king_squares_copy, 8);
            side = side_copy;
            enpassant = enpassant_copy;
            castle = castle_copy;
        }
        if(!foundLegalMove){
            //If check
            if(!is_square_attacked(king_squares[side], !side))  {
                return 0;
            }
            return -888888-depth;
        }
        return maxEval;
    } else {
        int minEval = 9999999;

        //Loop over generated moves
        int foundLegalMove = 0;
        for(int move_count = 0; move_count < size; move_count++) {
            //Make move
            int board_copy[BOARD_SIZE];
            int side_copy;
            int enpassant_copy;
            int castle_copy;
            int king_squares_copy[2];

            //Copy
            memcpy(board_copy, board, BOARD_MEM_SIZE);
            memcpy(king_squares_copy, king_squares, 8);
            side_copy = side;
            enpassant_copy = enpassant;
            castle_copy = castle;

            int move = move_list->moves[move_count];
            
            if(!make_move(move, all_moves)) {
                continue;
            } 
            foundLegalMove = 1;
            int eval = minimax(initialDepth, depth -1, 1, alpha, beta, &line);
            if(eval < minEval) {
                minEval = eval;
            }
            if(eval < beta) {
                pline->argmove[0] = move;
                memcpy(pline->argmove + 1, line.argmove, line.cmove * SIZE_OF_INT);
                pline->cmove = line.cmove + 1;
                beta = eval;
            }
            if(beta <= alpha) {
                break;
            }

            //Restore board
            memcpy(board, board_copy, BOARD_MEM_SIZE);
            memcpy(king_squares, king_squares_copy, 8);
            side = side_copy;
            enpassant = enpassant_copy;
            castle = castle_copy;
        }
        if(!foundLegalMove){
            if(!is_square_attacked(king_squares[side], !side))  {
                return 0;
            }
            return 888888+depth;
        }  
        return minEval;
    }
}

int playGame(int level, char *fen) {
    if(fen) {
        parse_fen(fen);
    }
    //int gameStart = getTimeInMs();

    int playOn = 1;
    int PLY = level*2;
    int result = 0;
    while(playOn) {
        playOn = 0;

        //Init start time
        int start_time = getTimeInMs();
        printf("**********************\n");
        LINE line;

        float score = minimax(PLY, PLY, 1, -9999999, 9999999, &line) / 100.0;

        printf("Principal line: %.2f\n", score);
        for(int i=0; i< line.cmove; i++) {
            //printf("%d\n", line.argmove[i]);
            print_move(line.argmove[i],0);
            printf(" ");
        }
        printf("\nMaking move:    ");
        print_move(bestMoveWhite,1);
        //printf("Move code = %d\n", bestMoveWhite);

        int timeElapsed = getTimeInMs() - start_time;
        printf("Move time:      %.2fs\n", timeElapsed/1000.0);
        printf("Nodes:          %d\n", EVALS);
        printf("Nodes/sec:      %.0f\n", EVALS/(timeElapsed/1000.0));
        printf("**********************\n");
        EVALS = 0;
        if(bestMoveWhite) {
            make_move(bestMoveWhite, all_moves);
            if(is_square_attacked(king_squares[side], !side) && !areThereAnyLegalMoves()) {
                if(side == white) {
                    printf("0-1\n");
                    result = 2;
                } else {
                    printf("1-0\n");
                    result = 1;
                }
                playOn = 0;
            }
        } else {
            playOn = 0;
        }
    }
    return result;
    //int gameTime = getTimeInMs() - gameStart;
    //printf("Game time:      %.2fs\n", gameTime/1000.0);
}

int main(int argc, char** argv) {
    if (argv[1] && ! strcmp(argv[1], "test")) {
        //Init start time
        int start_time = getTimeInMs();

        test(20, perft_test2(1, start_position), "Starting position, depth 1");
        test(400, perft_test2(2, start_position), "Starting position, depth 2");
        test(8902, perft_test2(3, start_position), "Starting position, depth 3");
        test(197281, perft_test2(4, start_position), "Starting position, depth 4");
        test(4865609, perft_test2(5, start_position), "Starting position, depth 5");

        test(48, perft_test2(1, "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - "), "Chessprogramming pos 2, depth 1");
        test(2039, perft_test2(2, "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - "), "Chessprogramming pos 2, depth 2");
        test(97862, perft_test2(3, "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - "), "Chessprogramming pos 2, depth 3");
        test(4085603, perft_test2(4, "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - "), "Chessprogramming pos 2, depth 4");
        test(193690690, perft_test2(5, "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - "), "Chessprogramming pos 2, depth 5");

        test(14, perft_test2(1, "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - "), "Chessprogramming pos 3, depth 1");
        test(191, perft_test2(2, "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - "), "Chessprogramming pos 3, depth 2");
        test(2812, perft_test2(3, "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - "), "Chessprogramming pos 3, depth 3");
        test(43238, perft_test2(4, "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - "), "Chessprogramming pos 3, depth 4");
        test(674624, perft_test2(5, "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - "), "Chessprogramming pos 3, depth 5");
        test(11030083, perft_test2(6, "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - "), "Chessprogramming pos 3, depth 6");
        
        test(6, perft_test2(1, "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1"), "Chessprogramming pos 4, depth 1");
        test(264, perft_test2(2, "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1"), "Chessprogramming pos 4, depth 2");
        test(9467, perft_test2(3, "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1"), "Chessprogramming pos 4, depth 3");
        test(422333, perft_test2(4, "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1"), "Chessprogramming pos 4, depth 4");
        test(15833292, perft_test2(5, "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1"), "Chessprogramming pos 4, depth 5");
        
        test(44, perft_test2(1, "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8"), "Chessprogramming pos 5, depth 1");
        test(1486, perft_test2(2, "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8"), "Chessprogramming pos 5, depth 2");
        test(62379, perft_test2(3, "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8"), "Chessprogramming pos 5, depth 3");
        test(2103487, perft_test2(4, "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8"), "Chessprogramming pos 5, depth 4");
        test(89941194, perft_test2(5, "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8"), "Chessprogramming pos 5, depth 5");

        test(46, perft_test2(1, "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10"), "Chessprogramming pos 6, depth 1");
        test(2079, perft_test2(2, "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10"), "Chessprogramming pos 6, depth 2");
        test(89890, perft_test2(3, "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10"), "Chessprogramming pos 6, depth 3");
        test(3894594, perft_test2(4, "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10"), "Chessprogramming pos 6, depth 4");
        test(164075551, perft_test2(5, "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10"), "Chessprogramming pos 6, depth 5");
        
        printf("Tests used %dms.\n\n", getTimeInMs()-start_time);
    } else if(argv[1] && ! strcmp(argv[1], "newGame")) {
        int level=4;
        char source[4];
        char target[4];
        printf("Welcome to the Pettersen chess engine.\n");
        printf("Engine level (1-4): ");
        scanf("%d",&level);
            
        parse_fen(start_position);
        //parse_fen("8/1pp5/3k4/1P1P1p2/1P1R1PPp/8/6P1/1r3K2 w - - 1 34");

        int res = 0;
        while(res == 0) {
            if(side == black) {
                printf("From square: ");
                scanf("%s", source);
                printf("To square: ");
                scanf("%s", target);

                moves move_list[1];
                generate_moves(move_list);
                int size = move_list->count;

                for(int move_count = 0; move_count < size; move_count++) {
                    int moveEncooded = move_list->moves[move_count];
                    char *sourceCoord = square_to_coords[get_move_source(moveEncooded)];
                    char *targetCoord = square_to_coords[get_move_target(moveEncooded)];
                    int promotedPiece = get_move_piece(moveEncooded);
                    if(promotedPiece) {
                        int foundPiece = char_pieces[target[2]];
                        if(foundPiece != promotedPiece) {
                            continue;
                        } else {
                            target[strlen(target)-1] = 0;
                        }
                    }

                    if(!strcmp(targetCoord, target) && !strcmp(sourceCoord, source)) {
                        int moveStatus = make_move(moveEncooded, all_moves);
                        if(!moveStatus) {
                            printf("Illegal move. Terminating");
                            break;
                        } else {
                            if(is_square_attacked(king_squares[side], !side) && !areThereAnyLegalMoves()) {
                                if(side == white) {
                                    printf("0-1\n");
                                    res = 2;
                                } else {
                                    printf("1-0\n");
                                    res = 1;
                                }
                            }
                        }
                        break;
                    } 
                }
            } else {
                res = playGame(level,0);
            }
            
        }
    }
    else {
        playGame(4, start_position);
        //playGame(1, "r1b1r1k1/ppb2ppp/8/8/3n3q/3B4/PPPN1PPn/R1BQ2KR w - - 0 14");
        //parse_fen("r1b1r1k1/ppb2ppp/8/8/3n4/3B4/PPPN1PPq/R1BQ2K1 w - - 0 15");
        //int score = evaluate(board);
        //printf("Score = %d");

        //parse_fen("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10");
        //parse_fen(start_position);
        //parse_fen("4k3/4P3/8/8/R7/8/3qPb2/4K3 w - - 0 1");
        //parse_fen("3k4/R6R/8/8/8/8/8/8 w - - 0 1");
        //parse_fen("R3k3/7R/8/8/8/8/8/8 b - - 0 1"); //Game over, mate
        //parse_fen("4k3/R7/7R/8/8/8/8/1K6 w - - 0 1");
        //parse_fen("3k4/8/8/8/8/7R/R7/3K4 w - - 0 1");
        //parse_fen("r5rk/5p1p/5R2/4B3/8/8/7P/7K w - - 0 1"); //Mate in three
        //parse_fen("7R/r1p1q1pp/3k4/1p1n1Q2/3N4/8/1PP2PPP/2B3K1 w - - 1 0"); //Mate in four
        //playGame(4, "7R/r1p1q1pp/3k4/1p1n1Q2/3N4/8/1PP2PPP/2B3K1 w - - 1 0");
        //parse_fen("Q7/p1p1q1pk/3p2rp/4n3/3bP3/7b/PP3PPK/R1B2R2 b - - 0 1"); //Mate in four
        //parse_fen("7R/r1p1q1pp/3k4/1p1n1Q2/3N4/8/1PP2PPP/2B3K1 w - - 1 0"); //Mate in four
        //parse_fen("6k1/3b3r/1p1p4/p1n2p2/1PPNpP1q/P3Q1p1/1R1RB1P1/5K2 b - - 0 1"); //Mate in five - 301.65s
        //parse_fen("r2qkbnr/ppp1pppp/2b5/8/P1B5/1QP1P3/3P1PPP/RNB1K1NR b KQk - 3 10"); //Black should not caputre with queen
        //parse_fen("rn3rk1/pbppq1pp/1p2pb2/4N2Q/3PN3/3B4/PPP2PPP/R3K2R w KQ - 7 11"); //Mate in seven
        //parse_fen("8/r1r3pk/1N2pp2/3p4/P2QP1qp/1R6/2PB2P1/5RK1 w - - 8 41");
        //parse_fen("8/8/8/8/6k1/5q2/8/6K1 b - - 0 1"); //Avoid stalemate draw
        //parse_fen("1k6/8/2Q5/1K6/8/8/8/8 w - - 0 1"); //Avoid stalemate draw
        //parse_fen("8/8/3k4/8/8/3K4/8/3R4 w - - 0 1"); //Rooks vs king
        //parse_fen("8/3KP3/8/8/8/8/6k1/7q b - - 0 1"); //Q vs king and pawn
        //parse_fen("3n3k/rp4pp/1p5r/pPp1pp2/PpPp1p2/3PpPp1/RN2P1P1/QBNR1BK1 b - - 0 1"); 
        //parse_fen("1Bb3BN/R2Pk2r/1Q5B/4q2R/2bN4/4Q1BK/1p6/1bq1R1rb w - - 0 1"); //Hard mate in 1
        
        //parse_fen("3r1rk1/pbb4p/1q3ppP/1B1P4/4PR2/5N2/1Q3PP1/2R2K2 w - - 0 1");
        //parse_fen("8/8/8/8/3b4/4k3/2B2p2/5K1R w - - 10 65");
        //print_board();

        //parse_fen("r5k1/1p1bbppp/8/3p4/1P1P4/1p3N2/1PP2PPP/2B1K2R w K - 0 1"); //Good quiescene position, avoid c2c4 and loss of rook on c1, 8 ply
        //parse_fen("2rqk1r1/pp2bpBp/2b1p3/3p4/4n3/1N1QPN2/PPP2PPP/R3K2R w KQ - 0 1");
    }
    return 0;
}