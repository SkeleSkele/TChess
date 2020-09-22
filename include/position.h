#ifndef POSITION_H
#define POSITION_H

#include "types.h"
#include "move.h"

#include <vector>

class Position {
  public:
    Position();
    Position(std::string);

    void initPieces();
    void loadFEN(std::string);
    void printBoard();
    static void printBitBoard(U64);
    void makeMove(Move&);
    void unmakeMove(Move&);
    std::vector<Move> getLegalMoves();

    static void populateMaskArrays();

  private:
    /**
     * Bitboard representation:
     *    a  b  c  d  e  f  g  h
     *   +-----------------------+
     * 8 |56 57 58 59 60 61 62 63| 8
     * 7 |48 49 50 51 52 53 54 55| 7
     * 6 |40 41 42 43 44 45 46 47| 6
     * 5 |32 33 34 35 36 37 38 39| 5
     * 4 |24 25 26 27 28 29 30 31| 4
     * 3 |16 17 18 19 20 21 22 23| 3
     * 2 |8  9  10 11 12 13 14 15| 2
     * 1 |0  1  2  3  4  5  6  7 | 1
     *   +-----------------------+
     *    a  b  c  d  e  f  g  h
     */
    U64 bbs[12];

    /**
     * 76543210
     *   7: white O-O-O
     *   6: white O-O
     *   5: black O-O-O
     *   4: black O-O
     *   3: en passant flag (1 if possible)
     * 2-0: en passant file (irrelevant if bit 3 is 0)
     */
    U8 flags;
    Color player; 
    U16 clock;

    // Masks used for fast calculations
    static U64 attackOnEmpty[5][64];
    static U64 blockerMask[5][64];
    static U64 behindMask[64][64];

    // Constants for De Bruijn multiplication
    static U64 deb;
    static int debArray[64];

    // Functions for manipulating the board
    Color switchPlayer();
    void setEPFile(int);
    void setCastlingFlag(int, Color);
    void placePiece(Piece, int);
    void movePiece(Piece, int, int);
    Piece removePiece(int);
    Piece removePiece(Piece, int);

    // Functions for retrieving board information
    int getEPFile();
    Piece getPiece(int);
    U64 getAttackedSquares(Piece, int);
    U64 getAttackedSquares(Color);
    U64 getOccupied();
    U64 getOccupied(Color);
    bool inCheck(Color);
    bool isLegalMove(Move&);
    bool canCastle(Color, int);
    void addCastlingMoveIfAble(std::vector<Move>&, Color, int);
    
    // Functions for calculating the masks
    static U64 calculateRookAttackOnEmpty(int, int);
    static U64 calculateBishopAttackOnEmpty(int, int);
    static U64 calculateKingAttackOnEmpty(int, int);
    static U64 calculateKnightAttackOnEmpty(int, int);
    static U64 calculateRookBlockerMask(int, int);
    static U64 calculateBishopBlockerMask(int, int);
    static U64 calculateBehindMask(int, int, int, int);

    // Miscellaneous utility functions
    static void addPawnMoves(std::vector<Move>&, int, int);
    static bool inBounds(int, int);
    static int frToSquare(int, int);
    static std::string frToString(int, int);
    static int squareToRank(int);
    static int squareToFile(int);
    static std::string pieceGraphic(Piece);
    static Piece swapColor(Piece);
    static Piece makeColor(Piece, Color);
    static Color getColor(Piece);
    static Color oppositeColor(Color);
    static int bitscan(U64);
};

#endif
