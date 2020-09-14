#ifndef MOVE_H
#define MOVE_H

#include "types.h"

/* This class represents a move that a piece makes from one square to another.
 * Legality of the move is not assumed. A Move requires a from-square, a
 * to-square, and a MoveType (to indicate capture, castling, etc.)
 *
 * More information is required for a move to be unmade in a chess position:
 * the position's flags from before the move, the clock, and the identity of
 * any captured Piece. These fields will be populated when the position calls
 * makeMove.
 *
 * Note that for castling moves, the from- and to-squares should refer to the
 * king. The movement of the rook will be handled by the makeMove function
 * implicitly.
 */

class Move {
  public:
    Move();
    Move(U8, U8, MoveType);

    void setUnmakeInfo(U8, U16, Piece);
    U8 getFrom();
    U8 getTo();
    MoveType getType();
    Piece getCapturedPiece();
    U8 getFlags();
    U16 getClock();

    Piece getPromotedPiece();
    bool isCapture();
    int getCastlingDirection();

  private:
    // These fields are required to make the move.
    U8 from;
    U8 to;
    MoveType type;

    // Once the move is made, relevant information for unmaking the move will
    // be stored here.
    U8 savedFlags;
    U16 savedClock;
    Piece capturedPiece;
};

#endif
