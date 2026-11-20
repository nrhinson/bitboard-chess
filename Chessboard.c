#include <ctype.h> 
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

enum { WP, WN, WB, WR, WQ, WK, BP, BN, BB, BR, BQ, BK };
enum { Black, White };
enum BoardMask : uint64_t {
    FILE_A = 0x0101010101010101ULL,
    FILE_B = 0x0202020202020202ULL,
    FILE_C = 0x0404040404040404ULL,
    FILE_D = 0x0808080808080808ULL,
    FILE_E = 0x1010101010101010ULL,
    FILE_F = 0x2020202020202020ULL,
    FILE_G = 0x4040404040404040ULL,
    FILE_H = 0x8080808080808080ULL,

    NOT_A_FILE = 0xFEFEFEFEFEFEFEFEULL,  // every square except the a-file (~FILE_A)
    NOT_H_FILE = 0x7F7F7F7F7F7F7F7FULL,  // every square except the h-file (~FILE_H)
    NOT_AB_FILE = 0xFCFCFCFCFCFCFCFCULL,  // every square except the a- and b-files (~(FILE_A | FILE_B))
    NOT_GH_FILE = 0x3F3F3F3F3F3F3F3FULL,  // every square except the g- and h-files (~(FILE_G | FILE_H))

    RANK_1 = 0x00000000000000FFULL,
    RANK_2 = 0x000000000000FF00ULL,
    RANK_3 = 0x0000000000FF0000ULL,
    RANK_4 = 0x00000000FF000000ULL,
    RANK_5 = 0x000000FF00000000ULL,
    RANK_6 = 0x0000FF0000000000ULL,
    RANK_7 = 0x00FF000000000000ULL,
    RANK_8 = 0xFF00000000000000ULL,
};

// Forward declarations
int identifyPiece(uint64_t currentSpace, uint64_t board[12]);
int pieceBelongsToSide(int side, int piece);

// Read functions

int getYN(void) {
    char buffer[8];

    while (true) {
        if (fgets(buffer, sizeof buffer, stdin) == NULL) {
            printf("Exiting...\n");
            exit(0);
        }
        char c = buffer[0];
        if (c == 'Y') return 1;
        if (c == 'N') return 0;

        printf("Please respond with Y or N\n"); 
    }
}

uint64_t getSpace(void) {
    char buffer[8];

    while (true) {
        if (fgets(buffer, sizeof buffer, stdin) == NULL) {
            printf("Exiting...\n"); 
            exit(0);
        }
        // Typing 00 at any square prompt quits the game
        if (buffer[0] == '0' && buffer[1] == '0') {
            printf("Exiting...\n");
            exit(0);
        }
        // as "E2"
        char file_in = toupper((unsigned char)buffer[0]);
        char rank_in = buffer[1];
        uint64_t space = 0x0000000000000001ULL;
        for (int rank = '1'; rank <= '8'; rank++) {
            for (char file = 'A'; file <= 'H'; file++) {
                if (file_in == file && rank_in == rank) return space;
                space <<= 1;  // Bit shifts to next square.
            }
        }
        printf("Please respond with a valid square.\n");
    }
}

// Initalizations

void init_board(uint64_t board[12]) {
    board[WP] = 0x000000000000FF00ULL;
    board[WN] = 0x0000000000000042ULL;
    board[WB] = 0x0000000000000024ULL;
    board[WR] = 0x0000000000000081ULL;
    board[WQ] = 0x0000000000000008ULL;
    board[WK] = 0x0000000000000010ULL;
    board[BP] = 0x00FF000000000000ULL;
    board[BN] = 0x4200000000000000ULL;
    board[BB] = 0x2400000000000000ULL;
    board[BR] = 0x8100000000000000ULL;
    board[BQ] = 0x0800000000000000ULL;
    board[BK] = 0x1000000000000000ULL;
}

// Visual functions

void print_chessboard(uint64_t board[12]) {
    printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
    const char* piece_print[] = {"WP", "WN", "WB", "WR", "WQ", "WK", "BP", "BN", "BB", "BR", "BQ", "BK"};
    int pieceFound = false;  
    for (int rank = 8; rank >= 1; rank--) {
        printf("%d ", rank);

        for (char file = 1; file <= 8; file++) {
            // Initalize the current space. Note: we cannot use ordinary bit
            // shifing here (1 shift per loop) due to the way the board prints.
            // (lowkey had to look this one up)
            uint64_t currentSpace = 1ULL << ((rank - 1) * 8 + (file - 1));

            // Loops through all given uint64_t, masking them with the current
            // space If the & mask does not change the current space, this means
            // that Piece exists on that spot.
            for (int pieces = WP; pieces <= BK; pieces++) {
                if ((currentSpace & board[pieces]) == currentSpace) {
                    printf("%s ", piece_print[pieces]);
                    pieceFound = 1;
                    break;
                }
            }
            if (pieceFound == 0)
                printf(".. ");
            else
                pieceFound = 0;
        }
        printf("\n");
    }
    printf("  a  b  c  d  e  f  g  h\n");
}

void capturePiece(uint64_t currentSpace, uint64_t board[12]) {
    const char* piece_print[] = {"WP", "WN", "WB", "WR", "WQ", "WK",
                                 "BP", "BN", "BB", "BR", "BQ", "BK"};
    // Masks current space with all peices.
    for (int pieces = WP; pieces <= BK; pieces++) {
        if ((currentSpace & board[pieces])) {
            board[pieces] ^= currentSpace;  
            printf("\n%s was captured!\n\n",
                   piece_print[pieces]);  
            return;
        }
    }
}

void intro(void) {
    printf(
        "Welcome to bitboard chess!\n This is a two player chess game where "
        "you will play chess with black and white pieces.\n This is a demo "
        "version of bitboard chess, so en passant and castling are not "
        "available.\n The game will also not prevent you from inappropriately "
        "moving during check.\n Lastly, input \"00\" to exit the game when "
        "prompted to move. \n Begin game? (Y/N) ");
    if (!getYN()) {
        printf("Exiting...\n");
        exit(0);
    }
}

// builders

uint64_t whitePieces(uint64_t board[12]) {return board[WP] | board[WB] | board[WK] | board[WN] | board[WQ] | board[WR]; }

uint64_t blackPieces(uint64_t board[12]) {return board[BP] | board[BB] | board[BK] | board[BN] | board[BQ] | board[BR]; }

uint64_t allPieces(uint64_t board[12]) { return whitePieces(board) | blackPieces(board); }

uint64_t emptySquares(uint64_t board[12]) { return ~(whitePieces(board) | blackPieces(board));}

// Square helpers
// Both take a single-square bitboard and return 1-8 (same numbering as
// print_chessboard). Return 0 if the space is empty (no bit set).

int getRank(uint64_t space) {
    if (space == 0) return 0;
    int index = __builtin_ctzll(
        space);  // index (0-63) of the set bit: counts the zeros below it
    return (index >> 3) + 1;  //>> 3 is divide by 8: every rank is 8 bits
}

int getFile(uint64_t space) {
    if (space == 0) return 0;
    int index = __builtin_ctzll(space);
    return (index & 7) + 1;  //& 7 is mod 8: keeps the low 3 bits, the position within the rank
}

// Piece logic. Where the fun is!
// All return 1 on success, 0 on failure.

uint64_t whitePawnMoves(uint64_t pawns, uint64_t empty, uint64_t enemies) {
    //Checks if square in front if empty.
    uint64_t push1 = (pawns << 8) & empty; 
    //Checks if 2 jump move is available.
    uint64_t push2 = ((push1 & RANK_3) << 8) & empty;  
    // Adds top left and top right squares to captures
    uint64_t captures = ((pawns << 7) & NOT_H_FILE) | ((pawns << 9) & NOT_A_FILE); 
    //(captures & enemies) returns only vaild diagonal moves
    return push1 | push2 | (captures & enemies); 
}

/* Mirror image for black: shifts go down instead of up. */
uint64_t blackPawnMoves(uint64_t pawns, uint64_t empty, uint64_t enemies) {
    uint64_t push1 = (pawns >> 8) & empty;
    uint64_t push2 = ((push1 & RANK_6) >> 8) & empty;
    uint64_t captures = ((pawns >> 7) & NOT_A_FILE)    /* down-right */
                        | ((pawns >> 9) & NOT_H_FILE); /* down-left  */
    return push1 | push2 | (captures & enemies);
}

int movePawn(uint64_t pawn, uint64_t destSpace, uint64_t board[12]) {
    uint64_t white = whitePieces(board);
    uint64_t black = blackPieces(board);
    uint64_t empty = emptySquares(board);

    uint64_t isWhite = (pawn & board[WP]) != 0;
    uint64_t whiteMask = -isWhite; 

    uint64_t validMoves = (whitePawnMoves(pawn, empty, black) & whiteMask) | (blackPawnMoves(pawn, empty, white) & ~whiteMask);

    if ((validMoves & destSpace) == 0) return 0;

    capturePiece(destSpace, board);

    int pawnIndex = BP * (int)!isWhite;
    board[pawnIndex] ^= (pawn | destSpace);
    return 1;
}


int moveKnight(uint64_t knight, uint64_t destSpace, uint64_t board[12]) {
    // Set sepratly for readabilty
    uint64_t vaildKnightMoves = (knight << 17 & NOT_A_FILE) | (knight << 15 & NOT_H_FILE);  //"Far Top" left and right (towards Rank 8, two ranks up one file over)
    vaildKnightMoves |=(knight >> 17 & NOT_H_FILE) | (knight >> 15 & NOT_A_FILE);  //"Far Bottom" left and right (towradsrank 1, two ranks down one file over)
    vaildKnightMoves |= (knight << 10 & NOT_AB_FILE) | (knight << 6 & NOT_GH_FILE);  //"Close top" left and right (towards rank 1, one rank down, two files over )
    vaildKnightMoves |= (knight >> 10 & NOT_GH_FILE) | (knight >> 6 & NOT_AB_FILE);  //"Close bottom" left and right (towards rank 1, one rank up, two files over )

    // First creats a uint64 that contains only pieces in the way of the knights
    // vaild moves using &, then removes those pieces using ^
    int knightSide = pieceBelongsToSide(White, identifyPiece(knight, board));

    if (knightSide) {
        vaildKnightMoves ^= (vaildKnightMoves & whitePieces(board));
    } else {
        vaildKnightMoves ^= (vaildKnightMoves & blackPieces(board));
    }

    if ((vaildKnightMoves & destSpace) == 0) return 0;

    capturePiece(destSpace, board);
    int knightIndex = WN + 6 * !knightSide;
    board[knightIndex] ^= (knight | destSpace);
    return 1;
} 

int moveKing(uint64_t king, uint64_t destSpace, uint64_t board[12]) {
    //Add move above and below king
    uint64_t vaildKingMoves = king >> 8 | king << 8; 
    //Check moves to left of king. Ensure it does not loop.
    vaildKingMoves |= (king >> 7 | king << 1 | king << 9) & NOT_A_FILE;  
    //Check moves to right of king. Ensure it does not loop.
    vaildKingMoves |= (king >> 9 | king >> 1 | king << 7) & NOT_H_FILE;  
    int kingSide = pieceBelongsToSide(White, identifyPiece(king, board));

    if (kingSide) {vaildKingMoves ^= (vaildKingMoves & whitePieces(board));
    } else {vaildKingMoves ^= (vaildKingMoves & blackPieces(board));}

    if ((vaildKingMoves & destSpace) == 0) return 0;

    capturePiece(destSpace, board);
    int index = WK + 6 * !kingSide;
    board[index] ^= (king | destSpace);
    return 1;
}

int moveRook(uint64_t rook, uint64_t destSpace, uint64_t board[12]) {
    uint64_t vaildRookMoves = 0;  
    int rookSide = pieceBelongsToSide(White, identifyPiece(rook, board));
    uint64_t current;

    current = rook;

    // CHECKS SPACES TO RIGHT OF KNIGHT
    while (1) {
        if ((FILE_H & current) != 0) break;  
        current <<= 1;                       
        if ((current & allPieces(board)) !=0) { 
            if (rookSide == pieceBelongsToSide(Black, identifyPiece(current, board))) {
                vaildRookMoves |= current;
            }
            break;
        }
        vaildRookMoves |= current;
    }

    current = rook;

    // CHECKS SPACES TO LEFT OF KNIGHT
    while (1) {
        if ((FILE_A & current) != 0) break;  
        current >>= 1;                       
        if ((current & allPieces(board)) != 0) {  
            if (rookSide ==pieceBelongsToSide(Black, identifyPiece(current, board))) {
                vaildRookMoves |= current;
            }
            break;
        }
        vaildRookMoves |= current;
    }

    current = rook;
    // CHECKS SPACES ABOVE OF KNIGHT
    while (1) {
        if ((RANK_8 & current) != 0) break; 
        current <<= 8;                       // move current 1 space up
        if ((current & allPieces(board)) !=
            0) {  // If there is a piece already at current
            // Adds move to list if capture is available, otherwise terminates
            // loop
            if (rookSide ==
                pieceBelongsToSide(Black, identifyPiece(current, board))) {
                vaildRookMoves |= current;
            }
            break;
        }
        vaildRookMoves |= current;
    }

    current = rook;
    // CHECKS SPACES TO BELOW OF KNIGHT
    while (1) {
        if ((RANK_1 & current) != 0) break; 
        current >>= 8;                       // move current 1 space down
        if ((current & allPieces(board)) != 0) {  // If there is a piece already at current adds move to list if capture is available, otherwise terminates loop
            if (rookSide == pieceBelongsToSide(Black, identifyPiece(current, board))) {
                vaildRookMoves |= current;
            }
            break;
        }
        vaildRookMoves |= current;
    }

    if ((vaildRookMoves & destSpace) == 0) return 0;

    capturePiece(destSpace, board);
    int index = identifyPiece( rook, board);  
    board[index] ^= (rook | destSpace);
    return 1;
}

int moveBishop(uint64_t bishop, uint64_t destSpace, uint64_t board[12]) {
    uint64_t vaildBishopMoves = 0;
    int bishopSide = pieceBelongsToSide(White, identifyPiece(bishop, board));
    uint64_t current;

    current = bishop;

    // CHECKS SPACES TO TOP RIGHT OF BISHOP
    while (1) {
        if (current & (FILE_H | RANK_8))
            break;      // already on the right edge or top edge
        current <<= 9;  // up 1 rank (+8), right 1 file (+1)
        if ((current & allPieces(board)) !=0) { if (bishopSide == pieceBelongsToSide(Black, identifyPiece(current, board))) {
                vaildBishopMoves |= current;
            }
            break;
        }
        vaildBishopMoves |= current;
    }

    current = bishop;

    // CHECKS SPACES TO TOP LEFT OF BISHOP
    while (1) {
        if (current & (FILE_A | RANK_8)) break;      // already on the left edge or top edge
        current <<= 7;  // up 1 rank (+8), left 1 file (-1)
        if ((current & allPieces(board)) != 0) {
            if (bishopSide ==
                pieceBelongsToSide(Black, identifyPiece(current, board))) {
                vaildBishopMoves |= current;
            }
            break;
        }
        vaildBishopMoves |= current;
    }

    current = bishop;

    // CHECKS SPACES TO BOTTOM RIGHT OF BISHOP
    while (1) {
        if (current & (FILE_H | RANK_1))
            break;      // already on the right edge or bottom edge
        current >>= 7;  // down 1 rank (-8), right 1 file (+1)
        if ((current & allPieces(board)) != 0) {
            if (bishopSide ==
                pieceBelongsToSide(Black, identifyPiece(current, board))) {
                vaildBishopMoves |= current;
            }
            break;
        }
        vaildBishopMoves |= current;
    }

    current = bishop;

    // CHECKS SPACES TO BOTTOM LEFT OF BISHOP
    while (1) {
        if (current & (FILE_A | RANK_1))
            break;      // already on the left edge or bottom edge
        current >>= 9;  // down 1 rank (-8), left 1 file (-1)
        if ((current & allPieces(board)) != 0) {
            if (bishopSide ==
                pieceBelongsToSide(Black, identifyPiece(current, board))) {
                vaildBishopMoves |= current;
            }
            break;
        }
        vaildBishopMoves |= current;
    }

    if ((vaildBishopMoves & destSpace) == 0) return 0;

    capturePiece(destSpace, board);
    int index = identifyPiece( bishop, board); 
    board[index] ^= (bishop | destSpace);  
    return 1;
}

int moveQueen(uint64_t queen, uint64_t destSpace, uint64_t board[12]) {
    return moveRook(queen, destSpace, board) || 
    moveBishop(queen, destSpace, board);
}

// Game logic

int identifyPiece(uint64_t currentSpace, uint64_t board[12]) {
    // Masks current space with all peices.
    for (int pieces = WP; pieces <= BK; pieces++) {
        if ((currentSpace & board[pieces]) == currentSpace) return pieces;
    }
    return -1;  // Returns -1 if no piece on square
}

int pieceBelongsToSide(int side, int piece) {
    if (side == White && piece >= WP && piece <= WK) return true;
    if (side == Black && piece >= BP && piece <= BK) return true;
    return false;
}
int movePiece(uint64_t currentSpace, uint64_t destSpace, uint64_t board[12]) {
    int piece = identifyPiece(currentSpace, board);
    if (piece == WP || piece == BP)
        return movePawn(currentSpace, destSpace, board);
    if (piece == WK || piece == BK)
        return moveKing(currentSpace, destSpace, board);
    if (piece == WN || piece == BN)
        return moveKnight(currentSpace, destSpace, board);
    if (piece == WB || piece == BB)
        return moveBishop(currentSpace, destSpace, board);
    if (piece == WR || piece == BR)
        return moveRook(currentSpace, destSpace, board);  
    if (piece == WQ || piece == BQ)
        return moveQueen(currentSpace, destSpace, board); 
    return 0; 
}

int takeTurn(int currentSide, uint64_t board[12]) {
    int spaceSelected = false;
    int piece;
    uint64_t currentSpace = 0; 
    if (currentSide == White)
        printf("White sides turn!\n"); 
    else
        printf("Black sides turn!\n"); 
    while (!spaceSelected) {
        while (!spaceSelected) {
            printf("Move the piece at: ");  
            currentSpace = getSpace();
            piece = identifyPiece(currentSpace, board);
            if (pieceBelongsToSide(currentSide, piece))
                spaceSelected = true;
            else
                printf("No piece of yours on this square!\n");
        }
        spaceSelected = false;
        printf("To the square at: ");
        uint64_t destSpace = getSpace();
        if (movePiece(currentSpace, destSpace, board))
            spaceSelected = true;  
        else
            printf("Not a valid move!\n");
    }
    // Following ensure game continues only while the opponent's king is still on the board.
    if (currentSide == White)
        return board[BK] != 0;
    else
        return board[WK] != 0;
}

// Main!

int main(void) {
    int active = true;
    intro();
    while (active) {
        uint64_t board[12];
        init_board(board);
        int gameRunning = true;
        int turn = White;
        while (gameRunning) {
            print_chessboard(board);  
            if (turn == White)
                gameRunning = takeTurn(turn, board);  // returns 0 if BK caught
            else
                gameRunning = takeTurn(turn, board);  // returns 0 if WK caught;
            turn = !turn;
        }
        if (turn)
            printf("Black won!\n");
        else
            printf("White won!\n");
        printf("Would you like to play again? (Y/N) ");
        active = getYN(); 
    }
    printf("Thank you for playing bitboard chess!\n");
}