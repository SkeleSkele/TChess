#ifndef MOVE_H
#define MOVE_H

#include "types.h"

class Move {
  public:
    Move();
    Move(U8, U8, MoveType);

    void setUnmakeInfo(U8, U16, Piece);
    U8 getFrom();
    U8 getTo();
    MoveType getType();

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
