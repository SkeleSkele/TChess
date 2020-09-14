#include <iostream>
#include <unistd.h>
#include <fstream>
#include <string>
#include <stack>

#include "position.h"
#include "types.h"
#include "move.h"

void testMakeMove(std::string);

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cout << "Please provide a test file." << std::endl;
    return 0;
  }
  
  testMakeMove(argv[1]);
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
  int lineNum = 2;
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

  // TODO: while moves are being tested, add them to a stack. Once all moves
  // have been tested, unmake all the moves by popping the stack.
  while (!moveStack.empty()) {
    std::cout << "Unmaking move" << std::endl;
    Move m = moveStack.top();
    p.unmakeMove(m);
    p.printBoard();
    moveStack.pop();
  }
  std::cout << "Test complete." << std::endl;
}
