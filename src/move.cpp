#include "move.h"
#include "types.h"

#include <iostream>

Move::Move() {
  from = 0;
  to   = 0;
  type = MoveType::QUIET;
  savedFlags = 0;
  savedClock = 0;
  capturedPiece = Piece::NO_PIECE;
}

Move::Move(U8 from, U8 to, MoveType type) {
  this->from = from;
  this->to   = to;
  this->type = type;
  savedFlags = 0;
  savedClock = 0;
  capturedPiece = Piece::NO_PIECE;
}

void Move::setUnmakeInfo(U8 flags, U16 clock, Piece piece) {
  savedFlags = flags;
  savedClock = clock;
  capturedPiece = piece;
}

U8 Move::getFrom() {
  return from;
}

U8 Move::getTo() {
  return to;
}

MoveType Move::getType() {
  return type;
}

Piece Move::getCapturedPiece() {
  return capturedPiece;
}

U8 Move::getFlags() {
  return savedFlags;
}

U16 Move::getClock() {
  return savedClock;
}

// If the move is a promotion, this will return the Piece corresponding to the
// new Piece. If it isn't a promotion, returns NO_PIECE.
Piece Move::getPromotedPiece() {
  // This move is a promotion
  if (type & 0x08) {
    // Get piece type
    Piece piece;
    switch (type & 0x03) {
      case 0:
        piece = W_KNIGHT;
        break;
      case 1:
        piece = W_BISHOP;
        break;
      case 2:
        piece = W_ROOK;
        break;
      case 3:
        piece = W_QUEEN;
    }
    // Change the color to black if destination is 1st rank
    if (to < 8u) {
      piece = (Piece)((int)piece + 6);
    }
    return piece;
  }
  // This move is not a promotion
  else 
    return Piece::NO_PIECE;
}

// Returns true if the move is some kind of capture
bool Move::isCapture() {
  return type & 0x04;
}

// Returns -1 if the move is a long castle, +1 if short castle, and 0 if it is
// not a castling move.
int Move::getCastlingDirection() {
  if (type == MoveType::LONG_CASTLE)
    return -1;
  else if (type == MoveType::SHORT_CASTLE)
    return 1;
  else
    return 0;
}

// For debugging, prints out from, to, piece type
void Move::debugPrint() {
  std::cout << (int)from << " " << (int)to << " " << (int)type << std::endl;
}
