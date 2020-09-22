// Position.cpp
#include "position.h"
#include "types.h"
#include "move.h"

#include <iostream>
#include <string>

// Static variables.
U64 Position::attackOnEmpty[5][64] = {};
U64 Position::blockerMask[5][64] = {};
U64 Position::behindMask[64][64] = {};

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
  loadFEN(fen);
}

// Resets the Position to the given FEN. 
void Position::loadFEN(std::string fen) {
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

// Prints out the given bitboard.
void Position::printBitBoard(U64 b) {
  for (int r = 7; r >= 0; r--) {
    for (int f = 0; f < 8; f++) {
      int sq = frToSquare(f, r);
      if (b & (ONE << sq)) 
        std::cout << "1 ";
      else
        std::cout << ". ";
    }
    std::cout << std::endl;
  }
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
    setCastlingFlag(-1, Color::WHITE);
  if (move.getFrom() == 7 || move.getTo() == 7)
    setCastlingFlag(1, Color::WHITE);
  if (move.getFrom() == 56 || move.getTo() == 56)
    setCastlingFlag(-1, Color::BLACK);
  if (move.getFrom() == 63 || move.getTo() == 63)
    setCastlingFlag(1, Color::BLACK);
  if (movingPiece == Piece::W_KING)
    setCastlingFlag(0, Color::WHITE);
  if (movingPiece == Piece::B_KING)
    setCastlingFlag(0, Color::BLACK);

  // Update en passant flag.
  if (move.getType() == MoveType::DOUBLE_PAWN_PUSH) {
    setEPFile(move.getFrom() % 8);
  }

  // Switch player.
  switchPlayer();

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

void Position::unmakeMove(Move& move) {
  // Switch player back
  switchPlayer();

  // Move Piece back where it came from
  Piece movedPiece = removePiece(move.getTo());
  if (move.getPromotedPiece() != Piece::NO_PIECE) {
    if (player == Color::WHITE)
      movedPiece = W_PAWN;
    else
      movedPiece = B_PAWN;
  }
  placePiece(movedPiece, move.getFrom());

  // Restore captured piece
  if (move.getType() == MoveType::EP_CAPTURE) {
    if (player == Color::WHITE)
      placePiece(Piece::B_PAWN, move.getTo() - 8);
    else
      placePiece(Piece::W_PAWN, move.getTo() + 8);
  }
  else if (move.isCapture())
    placePiece(move.getCapturedPiece(), move.getTo());

  // For castling moves, move the rook back
  if (move.getType() == MoveType::LONG_CASTLE) {
    if (player == Color::WHITE)
      movePiece(W_ROOK, 3, 0);
    else
      movePiece(B_ROOK, 59, 56);
  }
  else if (move.getType() == MoveType::SHORT_CASTLE) {
    if (player == Color::WHITE)
      movePiece(W_ROOK, 5, 7);
    else
      movePiece(B_ROOK, 61, 63);
  }

  // Restore flags and clock
  flags = move.getFlags();
  clock = move.getClock();
}

// Returns a vector containing all of the fully legal moves which could be
// made in the current position.
std::vector<Move> Position::getLegalMoves() {
  std::vector<Move> moves;

  U64 friends = getOccupied(player);
  U64 enemies = getOccupied(oppositeColor(player));
  U64 occupied = friends | enemies;

  // Pieces other than pawns
  for (int i = 0; i < 5; i++) {
    Piece p = (Piece)(i + 6*(int)player);
    // For each individual piece
    for (U64 b = bbs[p]; b != 0; b &= b - 1) {
      int from = bitscan(b);
      // For each of that piece's attacks
      for (U64 a = getAttackedSquares(p, from); a != 0; a &= a - 1) {
        int to = bitscan(a);
        U64 mask = ONE << to;
        MoveType mt = MoveType::QUIET;
        // Don't make a move if the target square is friendly
        if (friends & mask)
          continue;
        // Mark move as a capture if there is an enemy piece there
        else if (enemies & mask)
          mt = MoveType::CAPTURE;
        moves.push_back(Move(from, to, mt));
      }
    }
  }

  // Pawns
  Piece p = makeColor(Piece::W_PAWN, player);

  // Calculate square for potential en passant capture
  int epSquare = getEPFile();
  if (epSquare != -1)
    epSquare += (p == Piece::W_PAWN) ? 40 : 16;

  // For each individual pawn
  for (U64 b = bbs[p]; b != 0; b &= b - 1) {
    int from = bitscan(b);
    int file = squareToFile(from);
    int front = (p == Piece::W_PAWN) ? from + 8 : from - 8;
    // Single push forward
    if (~occupied & (ONE << front)) {
      addPawnMoves(moves, from, front);
      int doubleFront = (p == Piece::W_PAWN) ? from + 16 : from - 16;
      int homeRank = (p == Piece::W_PAWN) ? 1 : 6;
      int rank = squareToRank(from);
      // Check if double pawn push is possible
      if ((rank == homeRank) && (~occupied & (ONE << doubleFront)))
        moves.push_back(Move(from, doubleFront, MoveType::DOUBLE_PAWN_PUSH));
    }
    // Capture left
    if (file != 0) {
      int left = front - 1;
      if (left == epSquare) // ep capture left is possible
        moves.push_back(Move(from, left, MoveType::EP_CAPTURE));
      else if (enemies & (ONE << left)) // regular capture left is possible
        addPawnMoves(moves, from, left);
    }
    // Capture right
    if (file != 7) {
      int right = front + 1;
      if (right == epSquare) // ep capture right is possible
        moves.push_back(Move(from, right, MoveType::EP_CAPTURE));
      else if (enemies & (ONE << right)) // regular capture right is possible
        addPawnMoves(moves, from, right);
    }
  }

  // Add castling moves if appropriate
  if (!inCheck(player)) {
    addCastlingMoveIfAble(moves, player, -1);
    addCastlingMoveIfAble(moves, player, 1);
  }

  // Go through every move and exclude any which would leave the friendly king
  // in check
  std::vector<Move> legalMoves;
  for (unsigned int i = 0; i < moves.size(); i++)
    if (isLegalMove(moves[i]))
      legalMoves.push_back(moves[i]);
    

  return legalMoves;
}

// Switches whose player's turn it is and returns that value.
Color Position::switchPlayer() {
  if (player == Color::WHITE)
    player = Color::BLACK;
  else
    player = Color::WHITE;
  return player;
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

// Disables the castling rights on the given side and color. Use -1 for
// queenside, +1 for kingside, or 0 for both.
void Position::setCastlingFlag(int side, Color c) {
  // Disable long castle
  if (side <= 0) {
    if (c == Color::WHITE)
      flags &= 0x7f;
    else
      flags &= 0xdf;
  }
  // Disable short castle
  if (side >= 0) {
    if (c == Color::WHITE)
      flags &= 0xbf;
    else
      flags &= 0xef;
  }
}

// Updates bitboards to put the piece in the given square.
void Position::placePiece(Piece piece, int square) {
  if (piece == Piece::NO_PIECE)
    return;
  U64 mask = ONE << square;
  bbs[piece] |= mask;
}

// Moves the Piece from one square to the other
void Position::movePiece(Piece piece, int from, int to) {
  if (piece == Piece::NO_PIECE)
    return;
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
  if (piece == Piece::NO_PIECE)
    return Piece::NO_PIECE;
  U64 mask = ~(ONE << square);
  bbs[piece] &= mask;
  return piece;
}

// Returns the bitboard containing the attacks available to the given piece on
// the given square. Included in attacks are squares that a piece can move to
// according to its own movement rules, ignoring the possibility of leaving the
// friendly king in check, while also including captures of friendly pieces.
// This can be used both as part of move generation as well as for determining
// if a player is in check.
U64 Position::getAttackedSquares(Piece piece, int sq) {
  // No attacks for empty squares
  if (piece == Piece::NO_PIECE)
    return 0;

  if (piece == Piece::W_PAWN)
    return (ONE << (sq + 7)) | (ONE << (sq + 9));
  else if (piece == Piece::B_PAWN)
    return (ONE << (sq - 9)) | (ONE << (sq - 7));

  Piece p = makeColor(piece, Color::WHITE);

  // Blockers and beyond algorithm
  U64 a = attackOnEmpty[p][sq];
  for (U64 b = getOccupied() & blockerMask[p][sq]; b != 0; b &= (b - 1)) {
    int blockerSquare = bitscan(b);
    a &= ~behindMask[sq][blockerSquare];
  }

  return a;
}

// Like the above method, but this returns the union of all attacks available
// for the given player.
U64 Position::getAttackedSquares(Color c) {
  U64 a = 0;
  // For each piece type
  for (int i = 0; i < 6; i++) {
    Piece p = (Piece)(i + 6*(int)c);
    // For each piece of that type
    for (U64 b = bbs[p]; b != 0; b &= b - 1) {
      int sq = bitscan(b);
      a |= getAttackedSquares(p, sq);
    }
  }
  return a;
}

// Returns the union of all piece bitboards.
U64 Position::getOccupied() {
  U64 b = 0;
  for (int i = 0; i < 12; i++)
    b |= bbs[i];
  return b;
}

// Returns the union of all piece bitboards for the given color.
U64 Position::getOccupied(Color c) {
  U64 b = 0;
  for (int i = 0; i < 6; i++) {
    b |= bbs[i + 6*(int)c];
  }
  return b;
}

// Returns true if the given player is in check.
bool Position::inCheck(Color c) {
  // Locate the position of the friendly king
  Piece king = makeColor(Piece::W_KING, c);
  U64 k = bbs[king];

  // Make the union of all available enemy attacks
  U64 b = getAttackedSquares(oppositeColor(c));
  return (k & b) != 0;
}

// Returns true if the given move would not leave the friendly king in check.
// It is assumed that the move otherwise accords with the rules of piece
// movement in chess. 
bool Position::isLegalMove(Move& move) {
  makeMove(move);
  bool v = !inCheck(oppositeColor(player));
  unmakeMove(move);
  return v;
}

// Returns true if the given color can castle in the given direction (negative
// dir for queenside, positive or 0 for kingside)
bool Position::canCastle(Color c, int dir) {
  U8 mask = 0x10;
  if (c == Color::WHITE)
    mask <<= 2;
  if (dir < 0)
    mask <<= 1;
  return flags & mask;
}

// Adds castling for the given color and side (negative for O-O-O, nonnegative
// for O-O) to the vector of Moves if "able". Here "able" means precisely the
// following:
// 1. The appropriate castling flag is on.
// 2. There are no pieces between the king and the castling rook.
// 3. The intermediate square is not under attack by any enemy piece. (This is
// sufficient to know that the king does not "castle through check" given that
// he is not in check to begin with, see below)
//
// Note that this function does NOT consider whether the king is in check to
// begin with (caller should determine this), or if it will end up in check 
// (the move list will eventually be cleared of those sorts of moves)
void Position::addCastlingMoveIfAble(std::vector<Move>& v, Color c, int dir) {
  // Check if player has right to castle
  if (!canCastle(c, dir))
    return;

  // Check if the spaces between king and rook are vacant
  U64 mask;
  if (c == Color::WHITE && dir < 0) mask = 0x000000000000000e;
  else if (c == Color::WHITE && dir >= 0) mask = 0x0000000000000060;
  else if (c == Color::BLACK && dir < 0) mask = 0x0e00000000000000;
  else if (c == Color::BLACK && dir >= 0) mask = 0x6000000000000000;
  if (getOccupied() & mask)
    return;

  // Check if intermediate square is under attack
  int sq;
  if (c == Color::WHITE && dir < 0) sq = 3;
  else if (c == Color::WHITE && dir >= 0) sq = 5;
  else if (c == Color::BLACK && dir < 0) sq = 59;
  else if (c == Color::BLACK && dir >= 0) sq = 61;
  U64 a = getAttackedSquares(oppositeColor(c));
  if (a & (ONE << sq))
    return;

  // Move is okay, add it to the list
  if (c == Color::WHITE && dir < 0)
    v.push_back(Move(4, 2, MoveType::LONG_CASTLE));
  else if (c == Color::WHITE && dir >= 0)
    v.push_back(Move(4, 6, MoveType::SHORT_CASTLE));
  else if (c == Color::BLACK && dir < 0) 
    v.push_back(Move(60, 58, MoveType::LONG_CASTLE));
  else if (c == Color::BLACK && dir >= 0) 
    v.push_back(Move(60, 62, MoveType::SHORT_CASTLE));
}

// Populates the attackOnEmpty array
void Position::populateMaskArrays() {
  for (int f = 0; f < 8; f++) {
    for (int r = 0; r < 8; r++) {
      int sq = frToSquare(f, r);
      // Attack on otherwise empty bitboards
      U64 rook = calculateRookAttackOnEmpty(f, r);
      U64 bishop = calculateBishopAttackOnEmpty(f, r);
      attackOnEmpty[Piece::W_ROOK][sq] = rook;
      attackOnEmpty[Piece::W_BISHOP][sq] = bishop;
      attackOnEmpty[Piece::W_QUEEN][sq] = rook | bishop;
      attackOnEmpty[Piece::W_KING][sq] = calculateKingAttackOnEmpty(f, r);
      attackOnEmpty[Piece::W_KNIGHT][sq] = calculateKnightAttackOnEmpty(f, r);

      // "Blockers and beyond" bitboards
      U64 rookBlocker = calculateRookBlockerMask(f, r);
      U64 bishopBlocker = calculateBishopBlockerMask(f, r);
      blockerMask[Piece::W_ROOK][sq] = rookBlocker;
      blockerMask[Piece::W_BISHOP][sq] = bishopBlocker;
      blockerMask[Piece::W_QUEEN][sq] = rookBlocker | bishopBlocker;
      // std::cout << "Queen blockers on " << f << r << std::endl;
      // printBitBoard(queenBlocker);

      // Behind masks
      int fDir[8] = {-1, 0, 1, -1, 1, -1, 0, 1};
      int rDir[8] = {1, 1, 1, 0, 0, -1, -1, -1};
      for (int i = 0; i < 8; i++) {
        for (int toF = f + fDir[i], toR = r + rDir[i];
            inBounds(toF, toR);
            toF += fDir[i], toR += rDir[i]) {
          U64 b = calculateBehindMask(f, r, toF, toR);
          behindMask[sq][frToSquare(toF, toR)] = b;
        }
      }
    }
  }
}

// Generates the bitboard of rook attacks on an otherwise empty board with a
// rook on the given file and rank.
U64 Position::calculateRookAttackOnEmpty(int f, int r) {
  U64 b = 0;
  // Attacks along file
  for (int i = 0; i < 8; i++)
    b ^= ONE << (8*i + f);
  // Attacks along rank
  b ^= 0x00000000000000ffULL << (8*r);
  // Note that the rook's square gets xor'd twice and ends up as 0, as it
  // should.
  return b;
}

// Generates the bitboard of bishop attacks on an otherwise empty board with a
// bishop on the given file and rank
U64 Position::calculateBishopAttackOnEmpty(int f, int r) {
  U64 b = 0;
  int df[4] = {-1, 1, 1, -1};
  int dr[4] = {1, 1, -1, -1};
  // Loop for each of 4 directions
  for (int i = 0; i < 4; i++) {
    int tf = f;
    int tr = r;
    while (inBounds(tf += df[i], tr += dr[i])) {
      int sq = frToSquare(tf, tr);
      b |= ONE << sq;
    }
  }
  return b;
}

// Generates the bitboard of king attacks on an otherwise empty board with a
// king on the given file and rank
U64 Position::calculateKingAttackOnEmpty(int f, int r) {
  U64 b = 0;
  int df[8] = {-1, 0, 1, -1, 1, -1, 0, 1};
  int dr[8] = {1, 1, 1, 0, 0, -1, -1, -1};
  for (int i = 0; i < 8; i++) {
    if (inBounds(f + df[i], r + dr[i])) {
      int sq = frToSquare(f + df[i], r + dr[i]);
      b |= ONE << sq;
    }
  }
  return b;
}

// Generates the bitboard of knight attacks on an otherwise empty board with a
// knight on the given file and rank
U64 Position::calculateKnightAttackOnEmpty(int f, int r) {
  U64 b = 0;
  int df[8] = {-1, 1, -2, 2, -2, 2, -1, 1};
  int dr[8] = {2, 2, 1, 1, -1, -1, -2, -2};
  for (int i = 0; i < 8; i++) {
    if (inBounds(f + df[i], r + dr[i])) {
      int sq = frToSquare(f + df[i], r + dr[i]);
      b |= ONE << sq;
    }
  }
  return b;
}

// Calculates the "blockers and beyond" mask for a rook on the given file and
// rank.
U64 Position::calculateRookBlockerMask(int f, int r) {
  U64 b = 0;
  // Attacks along file, ignoring outermost two
  for (int i = 1; i <= 6; i++)
    b |= ONE << (8*i + f);
  // Attacks along rank, ignoring outermost two
  b |= 0x000000000000007eULL << (8*r);
  // Eliminate rook's own square
  int sq = frToSquare(f, r);
  b &= ~(ONE << sq);
  return b;
} 

// Calculates the "blockers and beyond" mask for a bishop on the given file and
// rank.
U64 Position::calculateBishopBlockerMask(int f, int r) {
  U64 b = calculateBishopAttackOnEmpty(f, r);
  return b &= 0x007e7e7e7e7e7e00ull;
}

// Calculates the "behind" mask from one square to another. This mask is all
// 1's along the ray from the "from" point to the "to" point, but only behind
// the "to" point. It is assumed that the two points can be connected by a
// queen move, unless the two points are equal, in which case this just returns
// 0.
U64 Position::calculateBehindMask(int fromF, int fromR, int toF, int toR) {

  if (fromF == toF && fromR == toR)
    return 0;

  int fDir = 0;
  int rDir = 0;
  if (fromR < toR) // north
    rDir = 1;
  else if (fromR > toR) // south
    rDir = -1;
  if (fromF < toF) // east
    fDir = 1;
  else if (fromF > toF) // west
    fDir = -1;

  U64 b = 0;
  for (int f = toF + fDir, r = toR + rDir; 
      inBounds(f, r); f += fDir, r += rDir) {
    b |= ONE << frToSquare(f,r);
  }
  return b;
}

// Given a to and from square, adds all of the possible pawn moves between the
// two. ie, it will either just add one move if it is not a promotion, or it
// will add four moves (one per promotion). Double pawn pushes and captures en
// passant should be handled some other way.
void Position::addPawnMoves(std::vector<Move>& moves, int from, int to) {
  bool isPromotion = false;
  bool isCapture = false;
  if (squareToRank(to) == 0 || squareToRank(to) == 7)
    isPromotion = true;
  if (squareToFile(from) != squareToFile(to))
    isCapture = true;

  // Promotions with Capture
  if (isPromotion && isCapture) {
    moves.push_back(Move(from, to, MoveType::KNIGHT_PROMOTION_CAPTURE));
    moves.push_back(Move(from, to, MoveType::BISHOP_PROMOTION_CAPTURE));
    moves.push_back(Move(from, to, MoveType::ROOK_PROMOTION_CAPTURE));
    moves.push_back(Move(from, to, MoveType::QUEEN_PROMOTION_CAPTURE));
  }
  // Promotions without Capture
  else if (isPromotion) {
    moves.push_back(Move(from, to, MoveType::KNIGHT_PROMOTION));
    moves.push_back(Move(from, to, MoveType::BISHOP_PROMOTION));
    moves.push_back(Move(from, to, MoveType::ROOK_PROMOTION));
    moves.push_back(Move(from, to, MoveType::QUEEN_PROMOTION));
  }
  // Captures without Promotion
  else if (isCapture)
    moves.push_back(Move(from, to, MoveType::CAPTURE));
  // Neither capture nor promotion
  else
    moves.push_back(Move(from, to, MoveType::QUIET));
}

// Returns if the given file-rank coordinate is inbounds
bool Position::inBounds(int f, int r) {
  return (f >= 0) && (f <= 7) && (r >= 0) && (r <= 7);
}

// Returns the square corresponding to the given file-rank (0 indexed)
int Position::frToSquare(int file, int rank) {
  return 8*rank + file;
}

// Returns the algebraic notation corresponding to the given file-rank
std::string Position::frToString(int f, int r) {
  return std::string(1, f + 'a') + std::string(1, r + '1');
}

// Converts the square index to its rank
int Position::squareToRank(int sq) {
  return sq / 8;
}

// Converts the square index to its file
int Position::squareToFile(int sq) {
  return sq % 8;
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
Piece Position::swapColor(Piece piece) {
  if (piece == Piece::NO_PIECE)
    return piece;
  return (Piece)(((int)piece + 6) % 12);
}

// Returns the Piece but with the given color.
Piece Position::makeColor(Piece piece, Color color) {
  if (piece == Piece::NO_PIECE)
    return piece;
  int i = (int)piece % 6;
  if (color == Color::WHITE)
    return (Piece)i;
  else
    return (Piece)(i + 6);
}

// Returns the Color of the given piece. Do not call this for a NO_PIECE, but
// if you do, it returns WHITE.
Color Position::getColor(Piece p) {
  if ((int)p < 6)
    return Color::WHITE;
  else
    return Color::BLACK;
}

// Returns the opposite of the given Color.
Color Position::oppositeColor(Color c) {
  if (c == Color::WHITE)
    return Color::BLACK;
  else
    return Color::WHITE;
}

U64 Position::deb = 0x03f79d71b4cb0a89;
int Position::debArray[64] = {
  0,  1, 48,  2, 57, 49, 28,  3,
  61, 58, 50, 42, 38, 29, 17,  4,
  62, 55, 59, 36, 53, 51, 43, 22,
  45, 39, 33, 30, 24, 18, 12,  5,
  63, 47, 56, 27, 60, 41, 37, 16,
  54, 35, 52, 21, 44, 32, 23, 11,
  46, 26, 40, 15, 34, 20, 31, 10,
  25, 14, 19,  9, 13,  8,  7,  6
};

// Returns the index of the least significant 1 bit in the given U64. Returns
// -1 if the U64 is all 0.
int Position::bitscan(U64 bb) {
  if (bb == 0) {
    std::cout << "ERROR: bitscan called with empty bitboard" << std::endl;
    return -1;
  }
  return debArray[((bb & -bb) * deb) >> 58];
}
