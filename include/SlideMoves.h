#pragma once

#include <array>
#include <memory>
#include <utility>

#include "Board.h"
#include "ChessTypes.h"

/*
could potentially implement a Proxy design pattern where you store the moveset
from the last time this was called, and if the last piece to move isn't relevant
for this piece's moveset then we just return the old one again.
*/

class SlideMovesInterface {
 protected:
  Board& _board;

 public:
  SlideMovesInterface(Board& board) : _board(board) {};
  virtual std::unique_ptr<newPieceMoves> computeMoves(int index) = 0;
};

class StraightLineMove : SlideMovesInterface {
  std::unique_ptr<newPieceMoves> computeMoves(int index);
};

class DiagonalMove : SlideMovesInterface {
  std::unique_ptr<newPieceMoves> computeMoves(int index);
};
