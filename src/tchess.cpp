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
void printHelp();
int playGame();
int bitscan(U64);

int main(int argc, char** argv) {
  // No arguments allowed
  if (argc != 1) {
    std::cout << "This program should not be run with arguments." << std::endl;
    argv[0] = argv[0]; // Suppress warning that argv is unused.
    return 0;
  }
  
  Position::populateMaskArrays();
  int result = playGame();
  if (result == 1)
    std::cout << "White won." << std::endl;
  else if (result == -1)
    std::cout << "Black won." << std::endl;
  else
    std::cout << "Drawn." << std::endl;
}

int playGame() {
  std::cout << "Starting a new game. Enter H for help." << std::endl;
  // Set up game board
  Position p;
  p.initPieces();
  bool drawOffered = false;
  bool drawAvailable = false;
  while (true) {
    p.printBoard();
    std::vector<Move> moves = p.getLegalMoves();
    p.nameMoves(moves);

    // Checkmate notification.
    bool check = p.inCheck();
    if (check && moves.size() == 0) {
      if (p.getPlayer() == Color::WHITE) {
        std::cout << "White has been checkmated." << std::endl;
        return -1;
      }
      else {
        std::cout << "Black has been checkmated." << std::endl;
        return 1;
      }
    }

    // Stalemate notification.
    if (moves.size() == 0) {
      if (p.getPlayer() == Color::WHITE)
        std::cout << "White is in stalemate." << std::endl;
      else
        std::cout << "Black is in stalemate." << std::endl;
      return 0;
    }

    // Draw notifications
    if (p.getClock() >= 100) {
      std::cout << "The 50-move rule has been enforced." << std::endl;
      return 0;
    }
    if (drawOffered)
      std::cout << "You have offered a draw." << std::endl;
    if (drawAvailable)
      std::cout << "You are eligible to claim a draw." << std::endl;

    // Check notification
    if (check)
      std::cout << "You are in check." << std::endl;

    // Get command
    std::cout << "Enter a command: " << std::endl;
    std::string response;
    std::cin >> response;

    // Check if user entered a move
    int m = p.lookupMove(response, moves);
    if (m != -1) {
      p.makeMove(moves[m]);
      drawAvailable = false;
      if (drawOffered) {
        drawOffered = false;
        drawAvailable = true;
      }
      continue;
    }

    // Check for other commands
    // Show move list
    if (response == "M" || response == "m") {
      std::cout << "These moves are available:" << std::endl;
      std::string moveString;
      for (unsigned int i = 0; i < moves.size(); i++) {
        moveString = moveString + moves[i].getName() + ",";
      }
      moveString.pop_back();
      std::cout << moveString << std::endl;
    }

    // Exit
    else if (response == "E" || response == "e")
      exit(EXIT_SUCCESS);

    // Show help menu
    else if (response == "H" || response == "h")
      printHelp();

    // Draw
    else if (response == "D" || response == "d") {
      if (drawAvailable)
        return 0;
      drawOffered = !drawOffered;
    }

    // Resign
    else if (response == "R" || response == "r") {
      if (p.getPlayer() == Color::WHITE)
        return -1;
      else
        return 1;
    }

    // Unrecognized command
    else {
      std::cout << "Unrecognized command. Enter H for help." << std::endl;
      std::cout<< "Note that moves are case sensitive." <<std::endl;
    }
  }
}

// Prints out the help menu.
void printHelp() {
  std::cout << "[M]oves - shows list of legal moves." << std::endl;
  std::cout << "[D]raw - offers or claims a draw." << std::endl;
  std::cout << "[R]esign - makes you resign." << std::endl;
  std::cout << "[H]elp - shows this menu." << std::endl;
  std::cout << "[E]xit - exits this program." << std::endl;
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
