# TChess

This is a command-line chess program written in C++. Currently, you can use it in place of a regular chess board to play a game. All of the piece-moving rules of chess as well as 50-move draws have been implemented. To compile with g++, run ```make``` in the top directory, and execute ```bin/main``` to run.

## Bitboard Representation Explanation
This program is meant to serve as the foundation for an engine which can play chess. Board representation and move generation were designed from the start to be as fast as possible. The board internally uses a <a href="https://www.chessprogramming.org/Bitboards">bitboard representation</a> and bitwise operations to generate moves and update the position. Each of the 12 different varieties of pieces use a 64 bit string where each index corresponds to a square on the board. If that bit is 1, then that piece is located there.

For example, the bitboard for white knights at the start of the game looks like this:
```
........
........
........
........
........
........
........
.1....1.
```

In order to calculate the squares which those knights can attack, we first locate the indices of the knights with a bitscan using <a href="https://www.chessprogramming.org/BitScan#De_Bruijn_Multiplication">De Bruijn multiplication</a>. Then for each knight index, we lookup an attack pattern based on that index. For example, the attacking mask for the knight on b1 looks like this.
```
........
........
........
........
........
1.1.....
...1....
........
```
All attack patterns from every starting index are calculated at the start of the program's execution so we can just look them up instead of calculate them "on the fly." Now we can determine the pseudolegal moves for that knight with the following bitwise operations:
```
knightMovesBB = knightAttackMask[b1] & ~(wPawnBB | wKnightBB | wBishopBB | wRookBB | wQueenBB | wKingBB)
```
("Pseudolegal" means that the move is legal except for the possibility that it would leave the friendly king in check.)

This is one example of how bitwise operations are used to speed up move processing. For sliding pieces (bishops, rooks, and queens) we use the so-called Blockers and Beyond algorithm described <a href="https://www.chessprogramming.org/Blockers_and_Beyond">here</a>.

## Board Display Explanation
When running the program, the board is displayed like this:
```
============================
    a* b  c  d  e  f  g  h*
  +------------------------+
8 |[r][n][b][q][k][b][n][r]| 8
7 |[p][p][p][p][p][p][p][p]| 7
6 |   :::   :::   :::   :::| 6
5 |:::   :::   :::   :::   | 5
4 |   :::   :::   :::   :::| 4
3 |:::   :::   :::   :::   | 3
2 |<P><P><P><P><P><P><P><P>| 2
1 |<R><N><B><Q><K><B><N><R>| 1
  +------------------------+
=>  a* b  c  d  e  f  g  h* (0)
============================
```
White pieces look like this: ```<P>```

Black pieces look like this: ```[p]```

Empty dark squares look like ```:::``` and light squares are just spaces.

The ```=>``` symbol indicates whose turn it is to move.

Asterisks on the ```a``` and ```h``` files indicate that the player still has castling rights in that direction. (ie: the rook and king have not moved away and returned to their home squares.)

Lastly, the number in parentheses ```(0)``` is the half-move clock, indicating how many turns have passed since the last capture or pawn move. When this clock reaches 100, the game will be drawn automatically.

## Planned Updates
- Loading and saving PGNs and FENs of games.
- Implementing repetition draws.
- A prettier interface, potentially a GUI.
- Position evaluation using negamax with alpha-beta pruning.
