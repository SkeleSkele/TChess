#ifndef TYPES_H
#define TYPES_H

#include <cstdint>
#include <string>

// Shorthand for unsigned integers.
typedef uint8_t  U8;
typedef uint16_t U16;
typedef uint64_t U64;

// More readable form of 1UL.
const U64 ONE = 1;

enum Color {
  WHITE = 0,
  BLACK = 1,
};

enum Piece {
  NO_PIECE = -1,
  W_KING,
  W_QUEEN,
  W_ROOK,
  W_BISHOP,
  W_KNIGHT,
  W_PAWN = 5,
  B_KING = 6,
  B_QUEEN,
  B_ROOK,
  B_BISHOP,
  B_KNIGHT,
  B_PAWN,
};


enum MoveType {
  // Bits 0,1 have no general meaning
  // Bit 2 is used for captures
  // Bit 3 is used for promotions
  QUIET = 0,
  DOUBLE_PAWN_PUSH = 1,
  SHORT_CASTLE = 2,
  LONG_CASTLE = 3,
  CAPTURE = 4,
  EP_CAPTURE = 5,
  // Note that 6, 7 are unused
  KNIGHT_PROMOTION = 8,
  BISHOP_PROMOTION = 9,
  ROOK_PROMOTION = 10,
  QUEEN_PROMOTION = 11,
  KNIGHT_PROMOTION_CAPTURE = 12,
  BISHOP_PROMOTION_CAPTURE = 13,
  ROOK_PROMOTION_CAPTURE = 14,
  QUEEN_PROMOTION_CAPTURE = 15,
};

#endif
