#ifndef POSITION_H
#define POSITION_H

#include "types.h"

class Position {
  public:
    Position();

    void initPieces();
    void printBoard();
    Piece getPiece(int);

    static std::string pieceGraphic(Piece);

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
    **/
    U64 bbs[12];
    U8 castling; // 0,1,2,3 -> WK, WQ, BK, BQ
    U8 epSquare; // -1 means no square
    Color player;
    U16 clock;
};

#endif
