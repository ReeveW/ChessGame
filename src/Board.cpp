

#include "Board.h"

#include <any>
#include <functional>
#include <iostream>
#include <optional>
#include <unordered_set>
#include <vector>

#include "Game.h"

std::array<Point, 8> sliders = {
    Point{0, -1}, {1, -1}, {1, 0}, {1, 1}, {0, 1}, {-1, 1}, {-1, 0}, {-1, -1},

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

std::array<Point, 8> knightDirections = {
    Point{1, -2}, {2, -1}, {2, 1}, {1, 2}, {-1, 2}, {-2, 1}, {-2, -1}, {-1, -2},

    /* visual aid for how knightDirections works:
     * _________________
     * |_|_|7|_|0|_|_|_|
     * |_|6|_|_|_|1|_|_|
     * |_|_|_|N|_|_|_|_|
     * |_|5|_|_|_|2|_|_|
     * |_|_|4|_|3|_|_|_|
     * |_|_|_|_|_|_|_|_|
     * |_|_|_|_|_|_|_|_|
     * |_|_|_|_|_|_|_|_|
     *
     * N is for knight, 0-7 denotes the possible moves in the order they're
     * stored in the array
     */
};

// std::array<Point, 4> pawnDirections = {
//     Point{0, 1},
//     {0, 2},
//     {-1, 1},
//     {1, 1},
//     /* visual aid for how pawnMoves works:
//      * _________________
//      * |_|_|_|1|_|_|_|_|
//      * |_|_|2|0|3|_|_|_|
//      * |_|_|_|P|_|_|_|_|
//      * |_|_|_|_|_|_|_|_|
//      * |_|_|_|_|_|_|_|_|
//      * |_|_|_|_|_|_|_|_|
//      * |_|_|_|_|_|_|_|_|
//      * |_|_|_|_|_|_|_|_|
//      *
//      * P is for pawn, 0-4 denotes possible moves in the order they're stored
//      in the array.
//      * 1 is only available if the pawn hasn't moved, 2 and 3 are only
//      possible when taking a piece or for en passant.
//      * All Point values are multiplied by -1 if the pawn is black, as it
//      moves the opposite way.
//      */
// };

/*
looks for pieces of the opposite colour and returns a list of their indices
*/
std::vector<int> Board::piecesOf(Colour turn) {
  std::vector<int> pieceLocations;
  forEachSquare([&](int i) {
    if (_board[i].colour != turn && _board[i].colour != Colour::Empty) {
      pieceLocations.push_back(i);
    }
  });
  return pieceLocations;
}

bool Board::getTurn() {
  if (lastPieceMoved <= 8 && lastPieceMoved != 0) {
    // false when blacks turn, true when whites
    return false;
  }
  return true;
}

bool Board::onSameLine(int from, int to, int direction) {
  int fromRow = from / 8, fromCol = from % 8;
  int toRow = to / 8, toCol = to % 8;

  switch (direction) {
    case 1:
    case -1:
      return fromRow == toRow;
    case 8:
    case -8:
      return fromCol == toCol;
    case 9:
    case -9:
      return (fromRow - fromCol) == (toRow - toCol);
    case 7:
    case -7:
      return (fromRow + fromCol) == (toRow + toCol);
  }
  return false;
}

void Board::newPin(int pinnedPiece, int kingIndex, int attackerIndex,
                   int direction) {
  pinInfo pin;
  pin.pinIndex = pinnedPiece;
  int start = kingIndex + direction;
  // Record path between pinned piece and king
  for (int currentSquare = start; currentSquare != attackerIndex + direction;
       currentSquare +=
       direction) {  // loop goes from 1 step past king to attackerIndex
    pin.pathToKing.insert(currentSquare);
  }
  pins.push_back(pin);
}

// don't need to revamp this as it's covered by slidemoves
void Board::slidingMoves(PieceData p, std::vector<MoveType>& moves,
                         const std::array<PieceData, 64>& currentBoard) {
  std::vector<int> directions;

  if (p.piece == Piece::Rook) {
    directions = {0, 2, 4, 6};  // rook directions
  } else if (p.piece == Piece::Bishop) {
    directions = {1, 3, 5, 7};  // bishop directions
  } else if (p.piece == Piece::Queen) {
    directions = {0, 1, 2, 3, 4, 5, 6, 7};  // queen directions
  }

  for (auto dir : directions) {
    int x = p.index % 8;
    int y = p.index / 8;

    while (true) {
      x += sliders[dir].X;
      y += sliders[dir].Y;

      if (x < 0 || x > 7 || y < 0 || y > 7) {
        break;
      }
      int i = x + (8 * y);

      if (currentBoard[i]) {
        if (canTake(i, p.isBlack, currentBoard)) {
          MoveType take{p.index, i, 2};
          moves.push_back(take);
        }
        break;
      }
      MoveType slide{p.index, i, 1};
      moves.push_back(slide);
    }
  }
}

std::vector<MoveType>& Board::knightMoves(
    PieceData p, int index, const std::array<PieceData, 64>& currentBoard) {
  for (auto nm : knightDirections) {
    int x = (index % 8) + nm.X;
    int y = (index / 8) + nm.Y;
    if (x < 0 || x > 7 || y < 0 || y > 7) {
      continue;
    }

    int i = x + (8 * y);

    if (currentBoard[i] && canTake(i, p.colour, currentBoard)) {
      MoveType take{index, i, 2};
      moves.push_back(take);
    } else if (!currentBoard[i]) {
      MoveType horse{p.index, i, 1};
      moves.push_back(horse);
    }
  }
}

void Board::pawnMoves(PieceData p, std::vector<MoveType>& moves,
                      const std::array<PieceData, 64>& currentBoard) {
  const int ADD_ROW = p.isBlack ? 8 : -8;
  int row = p.index / 8;
  int col = p.index % 8;
  ;

  int forwardOne = p.index + ADD_ROW;
  int forwardTwo = p.index + 2 * ADD_ROW;

  if (!currentBoard[forwardOne]) {
    MoveType f1{};
    f1.from = p.index;
    f1.to = forwardOne;
    if (p.isBlack && row == 6 || !p.isBlack && row == 1) {
      f1.typeOfMove = 5;  // means pawn is promoting
    } else {
      f1.typeOfMove = 1;
    }
    moves.push_back(f1);
    // Double move from starting position
    if ((p.isBlack && row == 1) || (!p.isBlack && row == 6)) {
      if (!currentBoard[forwardTwo]) {
        MoveType f2{p.index, forwardTwo, 1};
        moves.push_back(f2);
      }
    }
  }

  // Captures (normal and en passant)
  for (int dx : {-1, 1}) {
    int newCol = col + dx;
    if (newCol >= 0 && newCol < 8) {
      int captureIndex = p.index + ADD_ROW + dx;
      if (captureIndex >= 0 && captureIndex < 64) {
        if (currentBoard[captureIndex] &&
            canTake(captureIndex, p.isBlack, currentBoard)) {
          MoveType capture{};
          capture.from = p.index;
          capture.to = captureIndex;
          if (!p.isBlack && row == 1 || p.isBlack && row == 6) {
            capture.typeOfMove = 5;
          } else {
            capture.typeOfMove = 2;
          }
          moves.push_back(capture);
        }
        // En passant capture
        if (enPassantFile == newCol && row == 3 + p.isBlack) {
          if (enPassantLegalityCheck(p, moves, captureIndex)) {
            MoveType enPassant{p.index, captureIndex, 4};
            moves.push_back(enPassant);
          }
        }
      }
    }
  }
}

bool Board::enPassantLegalityCheck(PieceData p, std::vector<MoveType>& moves,
                                   int newIndex) {
  int directionToTakenPawn = 8;
  if (p.isBlack) {
    directionToTakenPawn = -8;
  }

  // simulating the en passant capture
  auto newBoard = _board;
  newBoard[newIndex] = 1 + p.isBlack * 8;
  newBoard[p.index] = 0;
  newBoard[newIndex + directionToTakenPawn] = 0;

  std::vector<MoveType> newSquares;
  std::vector<MoveType> attackingMoves;

  findAttackedSquares(newBoard, attackingMoves, p.isBlack * 8, !p.isBlack * 8);

  // to debug

  bool isLegal = !isInCheck();

  return isLegal;
}

int Board::pseudoLegalKingMoves(
    PieceData p, std::array<PieceData, 64>& moves,
    const std::array<PieceData, 64>&
        currentBoard) {  // ignore moves that would put king in check for now
  int castleSides = 0;
  for (auto km : sliders) {
    int x = (p.index % 8) + km.X;
    int y = (p.index / 8) + km.Y;
    if (x < 0 || x > 7 || y < 0 || y > 7) {
      continue;
    }

    int i = x + (8 * y);

    if (currentBoard[i] && canTake(i, p.isBlack, currentBoard)) {
      moves[i] = 2;
    } else if (!currentBoard[i]) {
      moves[i] = 1;
    }
  }
  if (p.isBlack) {
    if (castleRights[0]) {
      if (!currentBoard[1] && !currentBoard[2] && !currentBoard[3]) {
        moves[2] = 3;
        castleSides += 1;
      }
    }
    if (castleRights[1]) {
      if (!currentBoard[5] && !currentBoard[6]) {
        moves[6] = 3;
        castleSides += 2;
      }
    }
  } else {
    if (castleRights[2]) {
      if (!currentBoard[57] && !currentBoard[58] && !currentBoard[59]) {
        moves[58] = 3;
        castleSides += 1;
      }
    }
    if (castleRights[3]) {
      if (!currentBoard[61] && !currentBoard[62]) {
        moves[62] = 3;
        castleSides += 2;
      }
    }
  }
  return castleSides;
}

/*
checks if king can castle, and checks if his pseudo legal moves are being
attacked (remove from moveset if attacked)
*/
std::vector<MoveType>& Board::kingMoves(std::vector<MoveType>& legalKingMoves) {
  std::array<int, 64> legalSquares = {};
  int castleSides = pseudoLegalKingMoves(king, legalSquares, _board);
  forEachSquare([&](int i) {
    if (legalSquares[i] && squaresBeingAttacked[i]) {
      legalSquares[i] = 0;
    }
  });

  // queenside
  if ((castleSides & 1) &&  // single ampersand is to grab x (this case being 1)
                            // bit and see if its true or not
      (squaresBeingAttacked[3 + (56 * !king.isBlack)] ||
       squaresBeingAttacked[2 + (56 * !king.isBlack)] ||
       squaresBeingAttacked[4 + (56 * !king.isBlack)] || inCheck)) {
    legalSquares[2 + (56 * !king.isBlack)] = 0;
  }

  // king side
  if ((castleSides & 2) &&
      (squaresBeingAttacked[5 + (56 * !king.isBlack)] ||
       squaresBeingAttacked[6 + (56 * !king.isBlack)] || inCheck)) {
    legalSquares[6 + (56 * !king.isBlack)] = 0;
  }

  forEachSquare([&](int i) {
    if (legalSquares[i]) {
      MoveType kingMove{king.index, i, legalSquares[i]};
      legalKingMoves.push_back(kingMove);
    }
  });
}

/*
resets squaresBeingAttacked, then simulates all moves by enemy pieces using the
chessBoard passed, using attacks() to insert the moves into squaresBeingAttacked
*/
void Board::findAttackedSquares(const std::array<PieceData, 64>& chessBoard,
                                std::vector<MoveType>& attackingMoves,
                                Colour colour) {
  squaresBeingAttacked = {};
  forEachSquare([&](int i) {
    if (chessBoard[i].colour != colour) {
      attacks(chessBoard, attackingMoves, i);
    }
  });
}

/*
checks if king is currently under attack using squaresBeingAttacked
*/
bool Board::isInCheck() {  // this function is being called after all the moves
                           // have been generated i think?
  if (squaresBeingAttacked[king.index]) {
    return true;
  }
  return false;
}

void Board::findAttackersOnKing(const std::vector<MoveType>& attackingMoves,
                                bool& multipleAttackers,
                                std::optional<MoveType>& checkingMove) {
  multipleAttackers = false;
  for (MoveType move : attackingMoves) {
    if (move.to == king.index) {
      if (checkingMove) {
        multipleAttackers = true;
      } else {
        checkingMove = {move};
      }
    }
  }
}

/*
pretty sure there's a bug here?
*/
void Board::restrictMoves(
    std::optional<MoveType>
        checkingMove) {  // blocking checks doesn't work properly anymore
  int checkerIndex = checkingMove->from;
  Piece id = _board[checkerIndex].piece;
  blockingSquares.insert(
      checkerIndex);  // attacker can always just be taken by a piece
  if (id == Piece::Pawn || id == Piece::Knight) {
    // disallow all piece moves except king and taking the checker
    return;
  }
  int checkerRow = checkerIndex / 8;
  int checkerCol = checkerIndex % 8;
  int kingRow = king.index / 8;
  int kingCol = king.index % 8;
  int rowDirection;
  int colDirection;
  if (kingRow > checkerRow) {
    rowDirection = -1;
  } else if (kingRow == checkerRow) {
    rowDirection = 0;
  } else {
    rowDirection = 1;
  }
  if (kingCol > checkerCol) {
    colDirection = -1;
  } else if (kingCol == checkerCol) {
    colDirection = 0;
  } else {
    colDirection = 1;
  }
  int row = kingRow + rowDirection;
  int col = kingCol + colDirection;

  int i = 0;

  while (row != checkerRow || col != checkerCol) {
    i++;

    blockingSquares.insert(row * 8 + col);
    row += rowDirection;
    col += colDirection;
  }
}

void Board::attacks(const std::array<PieceData, 64>& chessBoard,
                    std::vector<MoveType>& attackingMoves, int colour,
                    int index) {
  int type = chessBoard[index] - colour;
  if (type == 1) {
    pawnAttacks(attackingMoves, colour, index);
  } else if (type == 2) {
    rookAttacks(chessBoard, attackingMoves, index);
  } else if (type == 3) {
    knightAttacks(attackingMoves, index);
  } else if (type == 4) {
    bishopAttacks(chessBoard, attackingMoves, index);
  } else if (type == 5) {
    queenAttacks(chessBoard, attackingMoves, index);
  } else if (type == 6) {
    kingAttacks(attackingMoves, index);
  }
}

/*
adds a pawn's attacks to squaresBeingAttacked. Remember pawn attacks and moves
are different.
*/
void Board::pawnAttacks(std::vector<MoveType>& attackingMoves, int colour,
                        int index) {
  int row = -8;
  if (colour) {
    row = 8;
  }
  if (index % 8 != 0) {
    squaresBeingAttacked[index - 1 + row] = 1;
    MoveType pawnAttack{index, index - 1 + row, MoveCategories::Regular};
    attackingMoves.push_back(pawnAttack);
  }
  if (index % 8 != 7) {
    squaresBeingAttacked[index + 1 + row] = 1;
    MoveType pawnAttack{index, index + 1 + row, MoveCategories::Regular};
    attackingMoves.push_back(pawnAttack);
  }
}

/*
adds a knight's attacks to squaresBeingAttacked.
*/
void Board::knightAttacks(std::vector<MoveType>& attackingMoves, int index) {
  for (Point kd : knightDirections) {
    int row = index / 8 + kd.Y;
    int col = index % 8 + kd.X;

    if (row >= 0 && row < 8 && col >= 0 &&
        col < 8) {  // if nothing is out of bounds
      int move = row * 8 + col;
      squaresBeingAttacked[move] = 1;
      MoveType knightAttack{index, move, MoveCategories::Regular};
      attackingMoves.push_back(knightAttack);
    }
  }
}

/*
adds a bishop's attacks to squaresBeingAttacked. uses sliders array for diagonal
movement.
*/
void Board::bishopAttacks(const std::array<int, 64>& chessBoard,
                          std::vector<MoveType>& attackingMoves, int index) {
  std::array directions = {1, 3, 5, 7};
  for (auto dir : directions) {
    int x = index % 8;
    int y = index / 8;

    while (true) {
      x += sliders[dir].X;
      y += sliders[dir].Y;

      if (x < 0 || x > 7 || y < 0 || y > 7) {
        break;
      }
      int i = x + (8 * y);

      if (chessBoard[i]) {
        squaresBeingAttacked[i] = 1;
        MoveType take{
            index, i,
            MoveCategories::Regular};  // basically the same as sliderMoves
                                       // except we mark attacks regardless of
                                       // the piece on the square
        attackingMoves.push_back(take);
        break;
      }
      squaresBeingAttacked[i] = 1;
      MoveType slide{index, i, MoveCategories::Regular};
      attackingMoves.push_back(slide);
    }
  }
}

/*
adds a rook's attacks to squaresBeingAttacked. uses sliders array for horizontal
and vertical movement.
*/
void Board::rookAttacks(const std::array<int, 64>& chessBoard,
                        std::vector<MoveType>& attackingMoves, int index) {
  std::array directions = {0, 2, 4, 6};
  for (auto dir : directions) {
    int x = index % 8;
    int y = index / 8;

    while (true) {
      x += sliders[dir].X;
      y += sliders[dir].Y;

      if (x < 0 || x > 7 || y < 0 || y > 7) {
        break;
      }
      int i = x + (8 * y);

      if (chessBoard[i]) {
        squaresBeingAttacked[i] = 1;
        MoveType take{
            index, i,
            MoveCategories::Regular};  // basically the same as sliderMoves
                                       // except we mark attacks regardless of
                                       // the piece on the square
        attackingMoves.push_back(take);
        break;
      }
      squaresBeingAttacked[i] = 1;
      MoveType slide{index, i, MoveCategories::Regular};
      attackingMoves.push_back(slide);
    }
  }
}

/*
adds a queen's attacks to squaresBeingAttacked. uses sliders array for
horizontal, vertical and diagonal movement.
*/
void Board::queenAttacks(const std::array<int, 64>& chessBoard,
                         std::vector<MoveType>& attackingMoves, int index) {
  std::array directions = {0, 1, 2, 3, 4, 5, 6, 7};
  for (auto dir : directions) {
    int x = index % 8;
    int y = index / 8;

    while (true) {
      x += sliders[dir].X;
      y += sliders[dir].Y;

      if (x < 0 || x > 7 || y < 0 || y > 7) {
        break;
      }
      int i = x + (8 * y);

      if (chessBoard[i]) {
        squaresBeingAttacked[i] = 1;
        MoveType take{
            index, i,
            MoveCategories::Regular};  // basically the same as sliderMoves
                                       // except we mark attacks regardless of
                                       // the piece on the square
        attackingMoves.push_back(take);
        break;
      }
      squaresBeingAttacked[i] = 1;
      MoveType slide{index, i, MoveCategories::Regular};
      attackingMoves.push_back(slide);
    }
  }
}

/*
king attacks are calculated differently than king moves. This is because we're
calculating for the opponent's turn here, so we don't care if it's legal or not.
*/
void Board::kingAttacks(std::vector<MoveType>& attackingMoves, int index) {
  for (auto move : sliders) {
    int row = index / 8 + move.Y;
    int col = index % 8 + move.X;

    if (row >= 0 && row < 8 && col >= 0 && col < 8) {
      // if nothing is out of bounds
      int to = row * 8 + col;
      squaresBeingAttacked[to] = 1;
      MoveType kingMove{index, to};
      attackingMoves.push_back(kingMove);
    }
  }
}

/*
default constructor, initializes board to the default starting position, and
sets all possible castling options to true.
*/
Board::Board() : _board(defaultBoard), castleRights() {
  castleRights.fill(true);
}

/*
used when a piece is being taken by en passant. index is the index of the piece
being taken.
*/
void Board::takePiece(int index) { _board[index] = E; }

void Board::promotePawn(int prevIndex, int newIndex, PieceData piece) {
  _board[prevIndex] = E;
  _board[newIndex] = piece;
  // lastPieceMoved = piece; // Don't know what this is for
}

PieceData Board::getPiece(int index) const { return _board[index]; }

std::vector<MoveType> Board::legalMoves(
    int index, PieceData piece, std::array<PieceData, 64> currentBoard) {
  std::vector<MoveType> moves;
  PieceData p{getPiece(index)};

  if (p.piece == Piece::Queen || p.piece == Piece::Rook ||
      p.piece == Piece::Bishop) {
    moves = slidingMoves(p, index, currentBoard);
  } else if (p.piece == Piece::Knight) {
    moves = knightMoves(p, index, currentBoard);
  } else if (p.piece == Piece::Pawn) {
    moves = pawnMoves(p, index, currentBoard);
  } else if (p.piece == Piece::King) {
    moves = kingMoves(moves);
  }

  isPinned(index, p, moves);

  if (inCheck) {
    if (p.piece != Piece::King) {
      deleteNonBlockingMoves(moves);
    }
  }

  return moves;
}

/*
I don't like this function, but it's a temporary solution before I revamp this
entire file
*/
void Board::resetArrays() {
  allLegalMoves.clear();
  blockingSquares.clear();
}

/*
this function generates all legal moves for the pieces of a colour (turn).
it then adds them to the allLegalMoves set, and then checks for stalemates or
checkmates.
*/

// HERE
void Board::generateAllMoves(Colour turn) {
  for (int pieceIndex : piecesOf(turn)) {
    PieceMoves currentPiece;
    currentPiece.index = pieceIndex;
    currentPiece.moves = legalMoves(pieceIndex, _board[pieceIndex], _board);
    if (!currentPiece.moves.empty()) {
      allLegalMoves.push_back(currentPiece);
    }
  }
  if (allLegalMoves.empty()) {
    if (inCheck) {
      std::cout << std::endl << "Checkmate!" << std::endl;
    } else {
      std::cout << std::endl << "Stalemate!" << std::endl;
    }
  }
}

/*
given an index, we look for a piece's move list at that index. if we find one,
return it.
*/
std::vector<MoveType> Board::checkMove(int index) {
  for (const PieceMoves& piece : allLegalMoves) {
    if (piece.index == index) {
      return piece.moves;
    }
  }
  PieceMoves empty;
  return {};
}

// need to swap this so it checks colour that's opposite of current turn, since
// now we pass the current player's turn, whereas we previously passed the
// opposing player's turn
void Board::findPinsToKing(Colour turn) {
  pins.clear();

  int kingIndex = -1;
  forEachSquare([&](int i) {
    if (_board[i].piece == Piece::King) {
      kingIndex = i;
    }
  });

  std::array<int, 8> directions = {1, -1, 8, -8, 9, -9, 7, -7};
  for (int direction : directions) {
    int current = kingIndex + direction;
    int pinnedPiece = -1;

    while (current >= 0 && current < 64 &&
           onSameLine(kingIndex, current, direction)) {
      if (_board[current].piece == Piece::Empty) {
        current += direction;
        continue;
      }

      if (_board[current].colour == turn) {
        // piece of same colour
        if (pinnedPiece == -1) {
          pinnedPiece = current;
          current += direction;
        } else {
          break;
        }
      } else {
        // enemy piece
        Piece type = _board[current].piece;
        if ((direction == 1 || direction == -1 || direction == 8 ||
             direction == -8) &&
            (type == Piece::Rook || type == Piece::Queen)) {
          // same file (rook, queen)
          if (pinnedPiece != -1)
            newPin(pinnedPiece, kingIndex, current, direction);
        } else if ((direction == 9 || direction == -9 || direction == 7 ||
                    direction == -7) &&
                   (type == Piece::Bishop || type == Piece::Queen)) {
          // same diagonal(bishop, queen)
          if (pinnedPiece != -1)
            newPin(pinnedPiece, kingIndex, current, direction);
        }
        break;
      }
    }
  }
}

void Board::isPinned(int index, PieceData p, std::vector<MoveType>& moves) {
  for (pinInfo& pin : pins) {
    if (pin.pinIndex == index) {
      auto newMoves = moves;
      moves.clear();
      // we copy moves to newMoves, then clear moves so that we can fill it with
      // only pinned moves.
      for (MoveType pieceSteps : newMoves) {
        if (pin.pathToKing.count(
                pieceSteps.to)) {  // this is checking if the current move
                                   // (pieceSteps.to) of the piece we're looking
                                   // at (p) is in the stack containing the
                                   // moves of the pin.
          moves.push_back(pieceSteps);
        }
      }
    }
  }
}

/*
checks if piece matches the colour of the current player's turn, in which case they aren't able to take that piece
*/
int Board::canTake(int index, Colour colour,
                   const std::array<PieceData, 64>& currentBoard) const {
  if (colour ==
      currentBoard[index].colour) {
    return 0;
  }
  return 2;
}

void Board::setEnPassantFile(int column) { enPassantFile = column; }

void Board::updateBoard(int prevIndex, int newIndex) {
  PieceData temp = _board[prevIndex];
  _board[prevIndex] = E;
  _board[newIndex] = temp;
  lastPieceMoved = _board[newIndex];
}

void Board::canCastle() {
  castleRights[0] =
      (castleRights[0] && (getPiece(4) == 14) && getPiece(0) == 10);

  castleRights[1] = (castleRights[1] && getPiece(4) == 14 && getPiece(7) == 10);

  castleRights[2] = (castleRights[2] && getPiece(60) == 6 && getPiece(56) == 2);

  castleRights[3] = (castleRights[3] && getPiece(60) == 6 && getPiece(63) == 2);
}

// resets the opposing players side of the en passant square array at the end of
// each players turn.
void Board::resetEnPassant() { enPassantFile = -1; }

/*
finds whether king is in check, and if by multiple attackers, then delegates the
restriction of moves accordingly
*/
void Board::findCheckingMoves() {
  bool turn = getTurn();
  int colour = !turn * 8;  // 8 when black, 0 when white
  int oppositeColour = turn * 8;
  bool multipleAttackers = false;

  std::vector<MoveType> attackingMoves;
  attackingMoves.reserve(64);

  findAttackedSquares(_board, attackingMoves, colour, oppositeColour);

  inCheck = isInCheck();  // pass moveType of attacker, and multipleAttackers
  if (inCheck) {
    std::optional<MoveType> checkingMove;
    findAttackersOnKing(attackingMoves, multipleAttackers, checkingMove);
    if (multipleAttackers) {
      onlyKingToMove = true;
    } else {
      restrictMoves(checkingMove);
    }
  }
}

void Board::findKing() {
  bool turn = getTurn();
  forEachSquare([&](int i) {
    if (_board[i] == 6 + !turn * 8) {
      king.index = i;
      king.isBlack = !turn;
      king.type = 6;
    }
  });
}

/*
When given a vector of moves, deletes all moves that aren't currently blocking a
check. Should only be called when king is in check.
*/
void Board::deleteNonBlockingMoves(std::vector<MoveType>& moves) {
  auto tempMoves = moves;
  moves.clear();
  for (const auto& move : tempMoves) {
    if (blockingSquares.find(move.to) != blockingSquares.end()) {
      moves.push_back(move);
    }
  }
}

void Board::printBoard() {
  forEachSquare([&](int i) {
    std::cout << _board[i] << " ";
    if ((i + 1) % 8 == 0) {
      std::cout << std::endl;
    }
  });
}
