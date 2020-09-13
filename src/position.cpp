// Position.cpp
#include "position.h"
#include "types.h"
#include "move.h"

#include <iostream>
#include <string>

// Constructor. All bitboards are initially empty.
Position::Position() {
  for (int i = 0; i < 12; i++)
    bbs[i] = 0;
  flags = 0;
  player = Color::WHITE;
  clock = 0;
}

// Constructor which uses FEN to set everything up.
Position::Position(std::string fen) {
  for (int i = 0; i < 12; i++)
    bbs[i] = 0;

  // First token: piece locations
  int i = 0, file = 0, rank = 7;
  char c;
  while ((c = fen[i++]) != ' ') {
    Piece piece = Piece::NO_PIECE;
    switch (c) {
      case 'K': piece = Piece::W_KING; break;
      case 'Q': piece = Piece::W_QUEEN; break;
      case 'R': piece = Piece::W_ROOK; break;
      case 'B': piece = Piece::W_BISHOP; break;
      case 'N': piece = Piece::W_KNIGHT; break;
      case 'P': piece = Piece::W_PAWN; break;
      case 'k': piece = Piece::B_KING; break;
      case 'q': piece = Piece::B_QUEEN; break;
      case 'r': piece = Piece::B_ROOK; break;
      case 'b': piece = Piece::B_BISHOP; break;
      case 'n': piece = Piece::B_KNIGHT; break;
      case 'p': piece = Piece::B_PAWN; break;
      case '/': rank--; file = 0; break;
      default: file += c - '0';
    }
    // Add piece if nondigit and not '/'
    if (piece != Piece::NO_PIECE)
      placePiece(piece, frToSquare(file++, rank));
  }

  // Second token: player to move
  if (fen[i++] == 'w')
    player = Color::WHITE;
  else
    player = Color::BLACK;
  i++; // skip space

  // Third token: castling rights
  flags = 0;
  while ((c = fen[i++]) != ' ') {
    switch (c) {
      case 'Q': flags |= 0x80; break;
      case 'K': flags |= 0x40; break;
      case 'q': flags |= 0x20; break;
      case 'k': flags |= 0x10; break;
    }
  }

  // Fourth token: en passant target square
  c = fen[i++];
  if (c != '-') {
    setEPFile(c - 'a');
    i++; // rank of the token is unused
  }
  i++; // skip space

  // Fifth token: halfmove clock
  clock = 0;
  while ((c = fen[i++]) != ' ') {
    clock = (clock * 10) + (c - '0');
  }

  // Sixth token: fullmove clock, not used.
}

// Sets the Position to the initial game state.
void Position::initPieces() {
  flags = 0xF0;
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
  if (flags & 0x20)
    std::cout << "*";
  else
    std::cout << " ";
  std::cout << " b  c  d  e  f  g  h";
  if (flags & 0x10)
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
  if (flags & 0x80)
    std::cout << "*";
  else
    std::cout << " ";
  std::cout << " b  c  d  e  f  g  h";
  if (flags & 0x40)
    std::cout << "*";
  else
    std::cout << " ";
  std::cout << " (" << clock << ")" << std::endl;
  
  // En passant availability
  if (getEPFile() != -1) {
    std::cout << "ep available on file " << getEPFile() << std::endl;
  }
  std::cout << "============================" << std::endl;
}

// Actuates the given move, and also prepares that move object so that it can
// be unmade if necessary.
void Position::makeMove(Move& move) {
  // Remove captured piece, if applicable
  Piece capturedPiece;
  if (move.getType() == MoveType::EP_CAPTURE) { // en passant
    if (player == Color::WHITE)
      capturedPiece = removePiece(Piece::B_PAWN, move.getTo() - 8);
    else
      capturedPiece = removePiece(Piece::W_PAWN, move.getTo() + 8);
  }
  else if (move.isCapture()) // regular capture
    capturedPiece = removePiece(move.getTo());
  else // non-capture
    capturedPiece = Piece::NO_PIECE;

  // Save information which will be needed to unmake move
  move.setUnmakeInfo(flags, clock, capturedPiece);

  // Move the moving Piece to its destination
  Piece movingPiece = removePiece(move.getFrom());
  Piece prom = move.getPromotedPiece();
  if (prom != Piece::NO_PIECE)
    movingPiece = prom;
  placePiece(movingPiece, move.getTo());

  // For castling moves, move the rook also
  int c = move.getCastlingDirection();
  if (c != 0) {
    if (player == Color::WHITE) {
      if (c == -1)
        movePiece(W_ROOK, 0, 3);
      else
        movePiece(W_ROOK, 7, 5);
    }
    else {
      if (c == -1)
        movePiece(B_ROOK, 56, 59);
      else
        movePiece(B_ROOK, 63, 61);
    }
  }

  // Update castling flags if necessary.
  if (move.getFrom() == 0 || move.getTo() == 0)
    flags &= 0x7f;
  if (move.getFrom() == 7 || move.getTo() == 7)
    flags &= 0xbf;
  if (move.getFrom() == 56 || move.getTo() == 56)
    flags &= 0xdf;
  if (move.getFrom() == 63 || move.getTo() == 63)
    flags &= 0xef;
  if (movingPiece == Piece::W_KING)
    flags &= 0x3f;
  if (movingPiece == Piece::B_KING)
    flags &= 0xcf;

  // Update en passant flag.
  flags &= 0xf0;
  if (move.getType() == MoveType::DOUBLE_PAWN_PUSH) {
    U8 mask = 0x08 | (move.getFrom() % 8);
    flags |= mask;
  }

  // Switch player.
  player = (player + 1) % 2;

  // Increment clock, or reset it.
  if (movingPiece == Piece::W_PAWN || movingPiece == Piece::B_PAWN)
    clock = 0;
  else if (move.getPromotedPiece() != Piece::NO_PIECE)
    clock = 0;
  else if (move.isCapture())
    clock = 0;
  else
    clock++;
}

// Returns the 0-indexed file on which en-passant is possible, or -1 if en
// passant is unavailable.
int Position::getEPFile() {
  if ((flags & 0x08) == 0)
    return -1;
  else
    return flags & 0x07;
}

// Sets the flags so that en passant on the given file is possible. Passing a
// value not between 0 and 7 will disable en passant.
void Position::setEPFile(int f) {
  if (f < 0 || f > 7) {
    flags &= 0xf0;
    return;
  }
  flags |= (U8)f;
  flags |= 0x08;
}

// Updates bitboards to put the piece in the given square.
void Position::placePiece(Piece piece, int square) {
  U64 mask = ONE << square;
  bbs[piece] |= mask;
}

// Moves the Piece from one square to the other
void Position::movePiece(Piece piece, int from, int to) {
  removePiece(piece, from);
  placePiece(piece, to);
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

// Like getPiece, but also removes that piece from the bitboard.
Piece Position::removePiece(int square) {
  U64 mask = ONE << square;
  for (int i = 0; i < 12; i++) {
    if (bbs[i] & mask) {
      bbs[i] ^= mask;
      return (Piece)i;
    }
  }
  return Piece::NO_PIECE;
}

// Faster than the above method, for use when you already know what the Piece
// is.
Piece Position::removePiece(Piece piece, int square) {
  U64 mask = ~(ONE << square);
  bbs[piece] &= mask;
  return piece;
}

// Returns the square corresponding to the given file-rank (0 indexed)
int Position::frToSquare(int file, int rank) {
  return 8*rank + file;
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

// Returns the Piece with the opposite color.
Piece swapColor(Piece piece) {
  if (piece == Piece::NO_PIECE)
    return piece;
  return (Piece)(((int)piece + 6) % 12);
}
