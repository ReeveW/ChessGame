#include "SlideMoves.h"
#include "ChessTypes.h"


std::array<std::pair<int, int>, 8> slides = {
    std::pair{0, -1}, {1, -1}, {1, 0},  {1, 1},
    {0, 1},           {-1, 1}, {-1, 0}, {-1, -1},

    /* visual aid for how translations works:
     *
     * _________________
     * |_|7|_|0|_|1|_|_|
     * |_|_|7|0|1|_|_|_|
     * |6|6|6|Q|2|2|2|2|
     * |_|_|5|4|3|_|_|_|
     * |_|5|_|4|_|3|_|_|
     * |5|_|_|4|_|_|3|_|
     * |_|_|_|4|_|_|_|3|
     * |_|_|_|4|_|_|_|_|
     *
     * Q is for queen, 0-7 denotes the possible squares she can move, in the
     * order they're stored in the translations array. the multiples of the
     * translations are also shown. from this table you can extrapolate what
     * rook moves would be (just even numbers), bishop moves (odd numbers) and
     * king moves (no multiples)
     */

};
std::array<std::pair<int, int>, 4> straightLineMoves = {
    {{1, -1}, {1, 1}, {-1, 1}, {-1, -1}}};

std::unique_ptr<newPieceMoves> StraightLineMove::computeMoves(int index) {
    std::unique_ptr<newPieceMoves> moveSet(new newPieceMoves());
  for (int i = 0; i < 4; ++i) {
    int x = index % 8;
    int y = index / 8;
    bool seeingThroughKing = false;
    while (true) {
      x += straightLineMoves[i].first;
      y += straightLineMoves[i].second;
      if (x < 0 || x > 7 || y < 0 || y > 7) {
        break;
      }
      int newIndex = x + (8 * y);
      newMoveType move = {index, newIndex, MoveCategories::Missing};
      if(seeingThroughKing){
        move.typeOfMove = MoveCategories::SeeThroughKing;
        break;
      }
      if(this->_board.getPiece(newIndex)){
        int attackedPiece = this->_board.getPiece(newIndex);
        if(attackedPiece){}
      }
      /*
      if (seeingThroughKing) {
        make newIndex's moveCategory seeThroughKing break;
      }
      if (board.pieceAt(newIndex)) {
        attackedPiece = board.pieceAt(newIndex);
        if (attackedPiece == enemyKing) {
          seeingThroughKing = true;
          moveCategory = capture;
        } else if (attackedPiece == sameColour) {
         moveCategory = defends
        } else {
         moveCategory = capture;
        }
      }else{
       moveCategory = regular;
      }
        */
    }
  }
}

//   std::array directions = {0, 2, 4, 6};
//   for (auto dir : directions) {
//     int x = index % 8;
//     int y = index / 8;

//     while (true) {
//       x += sliders[dir].X;
//       y += sliders[dir].Y;

//       if (x < 0 || x > 7 || y < 0 || y > 7) {
//         break;
//       }
//       int i = x + (8 * y);

//       if (chessBoard[i]) {
//         squaresBeingAttacked[i] = 1;
//         moveType take{index, i,
//                       1};  // basically the same as sliderMoves except we
//                       mark
//                            // attacks regardless of the piece on the square
//         attackingMoves.push_back(take);
//         break;
//       }
//       squaresBeingAttacked[i] = 1;
//       moveType slide{index, i, 1};
//       attackingMoves.push_back(slide);
//     }
//   }

std::array<std::pair<int, int>, 4> DiagonalMoves = {
    {{0, -1}, {1, 0}, {0, 1}, {-1, 0}}};

std::unique_ptr<newPieceMoves> DiagonalMove::computeMoves(int index) {}
