#include <ctype.h>  // FIX: needed for toupper() in getSpace
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

    // FIX: these were used by the pawn functions but never defined
    NOT_A_FILE =
        0xFEFEFEFEFEFEFEFEULL,  // every square except the a-file (~FILE_A)
    NOT_H_FILE =
        0x7F7F7F7F7F7F7F7FULL,  // every square except the h-file (~FILE_H)
    NOT_AB_FILE = 0xFCFCFCFCFCFCFCFCULL,  // every square except the a- and
                                          // b-files (~(FILE_A | FILE_B))
    NOT_GH_FILE = 0x3F3F3F3F3F3F3F3FULL,  // every square except the g- and
                                          // h-files (~(FILE_G | FILE_H))

    RANK_1 = 0x00000000000000FFULL,
    RANK_2 = 0x000000000000FF00ULL,
    RANK_3 = 0x0000000000FF0000ULL,
    RANK_4 = 0x00000000FF000000ULL,
    RANK_5 = 0x000000FF00000000ULL,
    RANK_6 = 0x0000FF0000000000ULL,
    RANK_7 = 0x00FF000000000000ULL,
    RANK_8 = 0xFF00000000000000ULL,
};

// Forward declarations: these are defined further down, but used earlier (e.g.
// in moveKnight)
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

        printf("Please respond with Y or N\n");  // FIX: missing semicolon
    }
}

uint64_t getSpace(void) {
    char buffer[8];

    while (true) {
        if (fgets(buffer, sizeof buffer, stdin) == NULL) {
            printf("Exiting...\n");  // FIX: missing semicolon
            exit(0);
        }
        // Typing 00 at any square prompt quits the game
        if (buffer[0] == '0' && buffer[1] == '0') {
            printf("Exiting...\n");
            exit(0);
        }
        // FIX: the board prints lowercase files (a-h), so accept "e2" as well
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
    const char* piece_print[] = {"WP", "WN", "WB", "WR", "WQ", "WK",
                                 "BP", "BN", "BB", "BR", "BQ", "BK"};
    int pieceFound = false;  // FIX: was true, so an empty a8 would print
                             // nothing and shift the row
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

// FIX: board is now passed in (it isn't a global), and the parameter is named
// currentSpace to match the body
void capturePiece(uint64_t currentSpace, uint64_t board[12]) {
    const char* piece_print[] = {"WP", "WN", "WB", "WR", "WQ", "WK",
                                 "BP", "BN", "BB", "BR", "BQ", "BK"};
    // Masks current space with all peices.
    for (int pieces = WP; pieces <= BK; pieces++) {
        if ((currentSpace & board[pieces])) {
            board[pieces] ^= currentSpace;  // FIX: typo "peices"
            printf("\n%s was captured!\n\n",
                   piece_print[pieces]);  // FIX: missing semicolon
            return;
        }
    }
}

void intro(void) {
    // FIX: missing semicolon (and a few spelling fixes in the text)
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

uint64_t whitePieces(uint64_t board[12]) {
    return board[WP] | board[WB] | board[WK] | board[WN] | board[WQ] |
           board[WR];
}
uint64_t blackPieces(uint64_t board[12]) {
    return board[BP] | board[BB] | board[BK] | board[BN] | board[BQ] |
           board[BR];
}
uint64_t allPieces(uint64_t board[12]) {
    return whitePieces(board) | blackPieces(board);
}
uint64_t emptySquares(uint64_t board[12]) {
    return ~(whitePieces(board) | blackPieces(board));
}

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
    return (index & 7) + 1;  //& 7 is mod 8: keeps the low 3 bits, the position
                             // within the rank
}

// Piece logic. Where the fun is!
// All return 1 on success, 0 on failure.

uint64_t whitePawnMoves(uint64_t pawns, uint64_t empty, uint64_t enemies) {
    uint64_t push1 = (pawns << 8) & empty; /* one square up, must be empty */
    uint64_t push2 = ((push1 & RANK_3) << 8) &
                     empty; /* two squares: only if push1 landed on
                               rank 3, i.e. pawn started on rank 2
                               AND the square in between was empty */
    uint64_t captures =
        ((pawns << 7) & NOT_H_FILE)    /* up-left  (mask stops a->h wrap)     */
        | ((pawns << 9) & NOT_A_FILE); /* up-right (mask stops h->a wrap)     */
    return push1 | push2 |
           (captures & enemies); /* captures only onto enemy pieces     */
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

    /* Which side is this pawn? Instead of if/else, turn the answer into a mask:
       isWhite is 1 or 0, and -isWhite is all 1s (0xFFFF...) or all 0s.
       This is two's complement: -1 in binary is every bit set.            */
    uint64_t isWhite = (pawn & board[WP]) != 0;
    uint64_t whiteMask = -isWhite; /* all 1s if white, all 0s if black */

    /* Compute both sides' moves, then keep only the one that applies. */
    uint64_t validMoves = (whitePawnMoves(pawn, empty, black) & whiteMask) |
                          (blackPawnMoves(pawn, empty, white) & ~whiteMask);

    /* The one place a branch makes sense: rejecting an illegal move. */
    if ((validMoves & destSpace) == 0) return 0;

    /* Remove any captured piece: clear the destination square on every board.
       (Clearing a bit that isn't set does nothing, so no check is needed.)  */
    capturePiece(destSpace, board);

    /* Move the pawn: XOR flips both the "from" and "to" bits in one step.
       WP is 0 and BP is 6, so the index is 6 for black and 0 for white.   */
    int pawnIndex = BP * (int)!isWhite;
    board[pawnIndex] ^= (pawn | destSpace);
    return 1;
}

// FIX: empty bodies in a function that returns int give garbage values, and
// moveRook had no body at all. These stubs now return 0 ("illegal move") until
// you write them.
int moveKnight(uint64_t knight, uint64_t destSpace, uint64_t board[12]) {
    // Set sepratly for readabilty
    uint64_t vaildKnightMoves =
        (knight << 17 & NOT_A_FILE) |
        (knight << 15 & NOT_H_FILE);  //"Far Top" left and right (towards Rank
                                      // 8, two ranks up one file over)
    vaildKnightMoves |=
        (knight >> 17 & NOT_H_FILE) |
        (knight >> 15 & NOT_A_FILE);  //"Far Bottom" left and right (towrads
                                      // rank 1, two ranks down one file over)
    vaildKnightMoves |=
        (knight << 10 & NOT_AB_FILE) |
        (knight << 6 & NOT_GH_FILE);  //"Close top" left and right (towards rank
                                      // 1, one rank down, two files over )
    vaildKnightMoves |=
        (knight >> 10 & NOT_GH_FILE) |
        (knight >> 6 & NOT_AB_FILE);  //"Close bottom" left and right (towards
                                      // rank 1, one rank up, two files over )

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
}  // TODO

int moveKing(uint64_t king, uint64_t destSpace, uint64_t board[12]) {
    // Set sepratly for readabilty
    uint64_t vaildKingMoves = king >> 8 | king << 8;  // Above and below
    vaildKingMoves |= (king >> 7 | king << 1 | king << 9) &
                      NOT_A_FILE;  // all spaces where the king moves right,
                                   // removes them if they loop.
    vaildKingMoves |= (king >> 9 | king >> 1 | king << 7) &
                      NOT_H_FILE;  // all spaces where the king moves left,
                                   // removes them if they loop.

    // First creats a uint64 that contains only pieces in the way of the knights
    // vaild moves using &, then removes those pieces using ^
    int kingSide = pieceBelongsToSide(White, identifyPiece(king, board));

    if (kingSide) {
        vaildKingMoves ^= (vaildKingMoves & whitePieces(board));
    } else {
        vaildKingMoves ^= (vaildKingMoves & blackPieces(board));
    }

    if ((vaildKingMoves & destSpace) == 0) return 0;

    capturePiece(destSpace, board);
    int index = WK + 6 * !kingSide;
    board[index] ^= (king | destSpace);
    return 1;
}

int moveRook(uint64_t rook, uint64_t destSpace, uint64_t board[12]) {
    uint64_t vaildRookMoves = 0;  // FIX: must start empty before |= adds to it
    int rookSide = pieceBelongsToSide(White, identifyPiece(rook, board));
    uint64_t current;

    current = rook;

    // CHECKS SPACES TO RIGHT OF KNIGHT
    while (1) {
        if ((FILE_H & current) != 0) break;  // FIX: parentheses (precedence)
        current <<= 1;                       // move current 1 space right
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

    // CHECKS SPACES TO LEFT OF KNIGHT
    while (1) {
        if ((FILE_A & current) != 0) break;  // FIX: parentheses (precedence)
        current >>= 1;                       // move current 1 space left
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
    // CHECKS SPACES ABOVE OF KNIGHT
    while (1) {
        if ((RANK_8 & current) != 0) break;  // FIX: parentheses (precedence)
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
        if ((RANK_1 & current) != 0) break;  // FIX: parentheses (precedence)
        current >>= 8;                       // move current 1 space down
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

    if ((vaildRookMoves & destSpace) == 0) return 0;

    capturePiece(destSpace, board);
    int index = identifyPiece(
        rook, board);  // FIX: was 'bishop', which doesn't exist in moveRook
    board[index] ^= (rook | destSpace);
    return 1;
}

int moveBishop(uint64_t bishop, uint64_t destSpace, uint64_t board[12]) {
    uint64_t vaildBishopMoves = 0;
    int bishopSide = pieceBelongsToSide(White, identifyPiece(bishop, board));
    uint64_t current;

    // Each diagonal step is one rank + one file:
    //   << 9 = up-right, << 7 = up-left, >> 7 = down-right, >> 9 = down-left
    // Before each step, stop if we're already on EITHER edge we're moving
    // toward. OR-ing the two edge masks lets one AND check both at once.

    current = bishop;

    // CHECKS SPACES TO TOP RIGHT OF BISHOP
    while (1) {
        if (current & (FILE_H | RANK_8))
            break;      // already on the right edge or top edge
        current <<= 9;  // up 1 rank (+8), right 1 file (+1)
        if ((current & allPieces(board)) !=
            0) {  // If there is a piece already at current
            // Adds move to list if capture is available, otherwise terminates
            // loop
            if (bishopSide ==
                pieceBelongsToSide(Black, identifyPiece(current, board))) {
                vaildBishopMoves |= current;
            }
            break;
        }
        vaildBishopMoves |= current;
    }

    current = bishop;

    // CHECKS SPACES TO TOP LEFT OF BISHOP
    while (1) {
        if (current & (FILE_A | RANK_8))
            break;      // already on the left edge or top edge
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
    int index = identifyPiece(
        bishop, board);  // whichever board this piece is on (bishop, or queen
                         // when called from moveQueen)
    board[index] ^= (bishop | destSpace);  // FIX: removed stray 'else'
    return 1;
}

int moveQueen(uint64_t queen, uint64_t destSpace, uint64_t board[12]) {
    return moveRook(queen, destSpace, board) ||
           moveBishop(queen, destSpace, board);
}

// Game logic

// FIX: board is now passed in (it isn't a global)
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
        return moveRook(currentSpace, destSpace,
                        board);  // FIX: was WP/BP (copy-paste)
    if (piece == WQ || piece == BQ)
        return moveQueen(currentSpace, destSpace,
                         board);  // FIX: was WP/BP (copy-paste)
    return 0;  // FIX: no piece found -> not a valid move (function had no
               // return here)
}

int takeTurn(int currentSide, uint64_t board[12]) {
    int spaceSelected = false;
    int piece;
    uint64_t currentSpace = 0;  // FIX: declared out here so it still exists
                                // after the inner loop ends
    if (currentSide == White)
        printf("White sides turn!\n");  // FIX: "white" -> "White", missing
                                        // semicolon
    else
        printf("Black sides turn!\n");  // FIX: missing semicolon
    while (!spaceSelected) {
        while (!spaceSelected) {
            printf("Move the piece at: ");  // FIX: missing semicolon
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
            spaceSelected = true;  // FIX: board argument was missing
        else
            printf("Not a valid move!\n");
    }

    // Game continues only while the opponent's king is still on the board.
    // An empty bitboard is exactly 0, so "king captured" is just board[K] == 0.
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
            print_chessboard(board);  // FIX: board argument was missing
            if (turn == White)
                gameRunning = takeTurn(
                    turn, board);  // Not implemented, returns 0 if BK caught
            else
                gameRunning = takeTurn(
                    turn, board);  // Not implemented, returns 0 if WK caught;
            turn = !turn;
        }
        if (turn)
            printf("Black won!\n");
        else
            printf("White won!\n");
        printf("Would you like to play again? (Y/N) ");
        active = getYN();  // FIX: active was never updated, so the game could
                           // never exit
    }
    printf("Thank you for playing bitboard chess!\n");
}