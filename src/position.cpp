// Position.cpp
#include "position.h"
#include "types.h"
#include <iostream>
#include <string>

// Constructor. All bitboards are initially empty.
Position::Position() {
  for (int i = 0; i < 12; i++)
    bbs[i] = 0;
  castling = 0;
  epSquare = -1;
  player = Color::WHITE;
  clock = 0;
}

// Sets the Position to the initial game state.
void Position::initPieces() {
  castling = 0x0F;
  epSquare = -1;
  player = Color::WHITE;
  clock = 0;

  bbs[0] = ONE << 4; // White King
  bbs[1] = ONE << 3; // White Queen
  bbs[2] = (ONE << 7) | 1; // White Rooks
  bbs[3] = (ONE << 2) | (ONE << 5); // White Bishops
  bbs[4] = (ONE << 1) | (ONE << 6); // White Knights
  bbs[5] = 0x000000000000FF00; // White Pawns 
  bbs[6] = ONE << 60; // Black King
  bbs[7] = ONE << 59; // Black Queen
  bbs[8] = (ONE << 56) | (ONE << 63); // Black Rooks
  bbs[9] = (ONE << 58) | (ONE << 61); // Black Bishops
  bbs[10] = (ONE << 57) | (ONE << 62); // Black Knights
  bbs[11] = 0x00FF000000000000; // Black Pawns
}

// Prints the board to the console.
void Position::printBoard() {
  // Upper boarder
  if (player == Color::BLACK)
    std::cout << "=>";
  else
    std::cout << "  ";
  std::cout << "  a";
  if (castling & 0x08)
    std::cout << "*";
  else
    std::cout << " ";
  std::cout << " b  c  d  e  f  g  h";
  if (castling & 0x04)
    std::cout << "*";
  else
    std::cout << " ";
  std::cout << std::endl;
  std::cout << "  +------------------------+" << std::endl;

  // Rows
  for (int rank = 7; rank >= 0; rank--) {
    // Left border
    std::cout << (rank + 1);
    std::cout << " |";
    // Print each piece
    for (int file = 0; file <= 7; file++) {
      int i = 8*rank + file;
      Piece p = getPiece(i);
      // Print empty square
      if (p == Piece::NO_PIECE) {
        if ((rank + file) % 2 == 1)
          std::cout << "   ";
        else
          std::cout << ":::";
      }
      // Print piece
      else
        std::cout << pieceGraphic(p);
    }
    // Right border
    std::cout << "| ";
    std::cout << (rank + 1);
    std::cout << std::endl;
  }

  // Bottom border
  std::cout << "  +------------------------+" << std::endl;
  if (player == Color::WHITE)
    std::cout << "=>";
  else
    std::cout << "  ";
  std::cout << "  a";
  if (castling & 0x02)
    std::cout << "*";
  else
    std::cout << " ";
  std::cout << " b  c  d  e  f  g  h";
  if (castling & 0x01)
    std::cout << "*";
  else
    std::cout << " ";
  std::cout << std::endl;
}

// Returns the Piece at the given square, which may be NO_PIECE
Piece Position::getPiece(int square) {
  U64 mask = ONE << square;
  for (int i = 0; i < 12; i++) {
    if (bbs[i] & mask) {
      return (Piece)i;
    }
  }
  return Piece::NO_PIECE;
}

// Returns the string representation of the Piece.
std::string Position::pieceGraphic(Piece piece) {
  switch (piece) {
    case W_KING:
      return "<K>";
    case W_QUEEN:
      return "<Q>";
    case W_ROOK:
      return "<R>";
    case W_BISHOP:
      return "<B>";
    case W_KNIGHT:
      return "<N>";
    case W_PAWN:
      return "<P>";
    case B_KING:
      return "[k]";
    case B_QUEEN:
      return "[q]";
    case B_ROOK:
      return "[r]";
    case B_BISHOP:
      return "[b]";
    case B_KNIGHT:
      return "[n]";
    case B_PAWN:
      return "[p]";
    default:
      return "";
  }
}
