#include <stdio.h>

//FEN debuging positions
char start_position[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
char tricky_position[] = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";

//piece encooding
enum pieces {e, P, N, B, R, Q, K, p, n, b, r, q, k, o};

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

//Castling writes
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

//unicode pieces
char *unicode_pieces[] = {".", "♟︎", "♞", "♝", "♜", "♛", "♚", "♙", "♘", "♗", "♖", "♕", "♔" };

//Chess board representation
int board[128] = {
    r, n, b, q, k, b, n, r,  o, o, o, o, o, o, o, o,
    p, p, p, p, p, p, p, p,  o, o, o, o, o, o, o, o,
    e, e, e, e, e, e, e, e,  o, o, o, o, o, o, o, o,
    e, e, e, e, e, e, e, e,  o, o, o, o, o, o, o, o,
    e, e, e, e, e, e, e, e,  o, o, o, o, o, o, o, o,
    e, e, e, e, e, e, e, e,  o, o, o, o, o, o, o, o,
    P, P, P, P, P, P, P, P,  o, o, o, o, o, o, o, o,
    R, N, B, Q, K, B, N, R,  o, o, o, o, o, o, o, o
};

//Side to move
int side = white;

//enpassant square
int enpassant = no_sq;

//Castling rights
int castle = 15; //dec 15 => bin 1111 => all castling available

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

    printf("    Board stats\n");
    printf("    Side to move: %s\n", (side == white) ? "white" : "black");
    printf("    Castling:     %c%c%c%c\n", (castle & KC) ? 'K':'-', 
                                    (castle & QC) ? 'Q':'-',
                                    (castle & kc) ? 'k':'-', 
                                    (castle & qc) ? 'q':'-' );
    printf("    Enpassant:    %s\n", (enpassant == no_sq) ? "n/a" : square_to_coords[enpassant]);

    printf("\n");
}

int is_attacked_by_sliding_piece(int square, int side, int offsets[], int white_piece, int black_piece) {
    int size = 4;
    for(int i = 0; i < size; i++) {
        int offset = offsets[i];
        int target_square = square + offset;
        int target_piece = board[target_square];

        while(!((target_square) & 0x88)) {
            if(side == white ? (target_piece == white_piece || target_piece == Q) : (target_piece == black_piece || target_piece == q)) {
                return 1;
            }
            if(target_piece) {
                break;
            }
            target_square += offset;
            target_piece = board[target_square];
        }
    }

    return 0;
}

int is_attacked_by_jumping_piece(int square, int side, int offsets[], int size, int white_piece, int black_piece) {
    for(int i = 0; i < size; i++) {
        int target_square = square + offsets[i];
        int target_piece = board[target_square];

        if(!(target_square & 0x88)) {
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
    
    if(is_attacked_by_jumping_piece(square, side, knight_offsets, 8, N, n)) {
        return 1;
    }

    if(is_attacked_by_jumping_piece(square, side, king_offsets, 8, K, k)) {
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

void print_move(int from_square, int to_square, int capture, int promotion) {
    if(promotion) {
        if(capture) {
            printf("%sx%sq\n", square_to_coords[from_square], square_to_coords[to_square]);
            printf("%sx%sr\n", square_to_coords[from_square], square_to_coords[to_square]);
            printf("%sx%sb\n", square_to_coords[from_square], square_to_coords[to_square]);
            printf("%sx%sn\n", square_to_coords[from_square], square_to_coords[to_square]);
        } else {
            printf("%s%sq\n", square_to_coords[from_square], square_to_coords[to_square]);
            printf("%s%sr\n", square_to_coords[from_square], square_to_coords[to_square]);
            printf("%s%sb\n", square_to_coords[from_square], square_to_coords[to_square]);
            printf("%s%sn\n", square_to_coords[from_square], square_to_coords[to_square]);
        }
    } else {
        if(capture) {
            printf("%sx%s\n", square_to_coords[from_square], square_to_coords[to_square]);
        } else {
            printf("%s%s\n", square_to_coords[from_square], square_to_coords[to_square]);
        }
    }
}

void generate_moves() {
    for(int square = 0; square < 128; square ++) {
        if(!(square & 0x88)) {

            //White pawn and castling moves
            if(side == white) {
                if(board[square] == P) {
                    //quiet
                    int to_square = square - 16;

                    //check if target square is on board
                    if(!(to_square & 0x88) && board[to_square] == e) {
                        
                        //pawn promotions
                        if(square >= a7 && square <= h7) {
                            print_move(square, to_square, 0, 1);
                        } else {
                            //one square ahead
                            print_move(square, to_square, 0, 0);

                            //two squares ahead
                            to_square -= 16;
                            if(square >= a2 && square <= h2 && board[to_square] == e) {
                                print_move(square, to_square, 0, 0);
                            }
                        }
                    }

                    //White pawn captures
                    to_square = square - 15;
                    if(!(to_square & 0x88) && ((board[to_square] >= p && board[to_square] <= q) || board[to_square] == enpassant)) {
                        if(square >= a7 && square <= h7) {
                            print_move(square, to_square, 1, 1);
                        } else {
                            print_move(square, to_square, 1, 0);
                        }
                    }
                    to_square = square - 17;
                    if(!(to_square & 0x88) && ((board[to_square] >= p && board[to_square] <= q) || board[to_square] == enpassant)) {
                        if(square >= a7 && square <= h7) {
                            print_move(square, to_square, 1, 1);
                        } else {
                            print_move(square, to_square, 1, 0);
                        }
                    }
                }
            } else {
                if(board[square] == p) {
                    int to_square = square + 16;

                    //check if target square is on board
                    if(!(to_square & 0x88) && board[to_square] == e) {
                        
                        //pawn promotions
                        if(square >= a2 && square <= h2) {
                            print_move(square, to_square, 0, 1);
                        } else {
                            //one square ahead
                            print_move(square, to_square, 0, 0);

                            //two squares ahead
                            to_square += 16;
                            if(square >= a7 && square <= h7 && board[to_square] == e) {
                                print_move(square, to_square, 0, 0);
                            }
                        }
                    }
                }
            }
        }
    }
}



int main() {
    parse_fen("r3k2r/pP1pqpb1/bn2pnp1/2pPN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq c6 0 1");
    print_board();
    generate_moves();
    //print_attached_squares(white);

    return 0;
}