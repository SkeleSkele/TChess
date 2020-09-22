#include "position.h"
#include "types.h"
#include "move.h"

#include <iostream>
#include <unistd.h>
#include <fstream>
#include <string>
#include <stack>
#include <vector>

void testMakeMove(std::string);
void testMoveGenAccuracy(std::string);
int bitscan(U64);

int main(int argc, char** argv) {
  // Run test file
  if (argc == 2) {
    Position::populateMaskArrays();
    testMoveGenAccuracy(argv[1]);
    return 0;
  }
  
  // No arg run
  Position::populateMaskArrays();
}

void testMoveGenAccuracy(std::string filename) {
  // Open file
  std::ifstream file(filename);
  if (!file.is_open()) {
    std::cout << "Unable to open file " << filename << std::endl;
    return;
  }

  std::string line;
  int lineNum = 0;
  while (getline(file, line)) {
    // Ignore empty lines and comments
    lineNum++;
    if (line.size() == 0 || line[0] == '#')
      continue;

    // Locate comma
    size_t comma = line.find(",");
    if (comma == std::string::npos) {
      std::cout << "Line formatting error: " << lineNum << std::endl;
      return;
    }

    // Create position from FEN
    Position p(line.substr(0, comma));

    // Count number of moves in position
    std::vector<Move> moves = p.getLegalMoves();
    int numMoves = moves.size();
    int expected = std::stoi(line.substr(comma + 1));
    if (numMoves != expected) {
      printf("Line %d: expected %d, got %d\n", lineNum, expected, numMoves);
      for (unsigned int i = 0; i < moves.size(); i++)
        moves[i].debugPrint();
      p.printBoard();
    }
  }
}

void testMakeMove(std::string filename) {
  // Open file
  std::ifstream file(filename);
  if (!file.is_open()) {
    std::cout << "Unable to open file " << filename << std::endl;
    return;
  }

  // Make a new board using FEN from line 1
  std::string line;
  getline(file, line);
  Position p(line);
  p.printBoard();

  // Store moves in a stack so we can unmake them later
  std::stack<Move> moveStack;
  
  // Loop through lines
  int lineNum = 1;
  while (getline(file, line)) {
    // Ignore empty lines and comments
    lineNum++;
    if (line.size() == 0 || line[0] == '#')
      continue;

    // Locate spaces
    size_t space1 = line.find(" ");
    size_t space2 = line.find(" ", space1 + 1);
    if (space1 == std::string::npos || space2 == std::string::npos) {
      std::cout << "Line formatting error: " << lineNum << std::endl;
      return;
    }

    // Populate values
    int from = std::stoi(line.substr(0, space1));
    int to = std::stoi(line.substr(space1 + 1, space2 - space1 - 1));
    int moveType = std::stoi(line.substr(space2 + 1));

    // Make move
    std::cout << "Making move " << lineNum << std::endl;
    Move m((U8)from, (U8)to, (MoveType)moveType);
    p.makeMove(m);
    moveStack.push(m);
    p.printBoard();
  }
  file.close();

  while (!moveStack.empty()) {
    std::cout << "Unmaking move" << std::endl;
    Move m = moveStack.top();
    p.unmakeMove(m);
    p.printBoard();
    moveStack.pop();
  }
  std::cout << "Test complete." << std::endl;
}

