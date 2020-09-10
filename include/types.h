#ifndef TYPES_H
#define TYPES_H

#include <cstdint>
#include <string>

typedef uint8_t  U8;
typedef uint16_t U16;
typedef uint64_t U64;

const U64 ONE = 1;

enum Color {
  WHITE = 0,
  BLACK = 1,
};

enum Piece {
  W_KING,
  W_QUEEN,
  W_ROOK,
  W_BISHOP,
  W_KNIGHT,
  W_PAWN,
  B_KING,
  B_QUEEN,
  B_ROOK,
  B_BISHOP,
  B_KNIGHT,
  B_PAWN,
  NO_PIECE = -1
};

#endif
