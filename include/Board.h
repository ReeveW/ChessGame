

#ifndef BOARD_H
#define BOARD_H

#include <array>
#include <functional>
#include <optional>
#include <unordered_set>
#include <vector>
#include "ChessTypes.h"

struct Point {  // struct to store coordinates on the board array
  int X, Y;  // all Y values get multiplied by 8 when converted to board indices
};
// struct pieceData {
//   bool isBlack;  // 0 is white, 1 is black
//   int type;
//   int index;
// };

struct pinInfo {
  int pinIndex;
  std::unordered_set<int> pathToKing;  // vector of indices which track the path
                                       // between the pinner and the king
};

class Board {
 private:
  std::array<PieceData, 64> _board;
  std::array<bool, 4>
      castleRights;  // index 0 is black queenside, 1 is black kingside, 2 is
                     // white queenside, 3 is white kingside
  // bool turn; // white's turn if false, black's if true
  int enPassantFile = -1;
  PieceData lastPieceMoved = E;
  bool onlyKingToMove = false;
  bool inCheck = false;
  PieceData king{Piece::King, Colour::White};
  std::vector<PieceMoves> allLegalMoves;
  std::vector<pinInfo> pins;
  std::unordered_set<int> blockingSquares;
  std::array<int, 64> squaresBeingAttacked = {};

  inline void forEachSquare(
      const std::function<void(int)>&
          action) {  // used a lot to loop through each square in the board
    for (int i = 0; i < 64; ++i) {
      action(i);
    }
  }
  std::vector<int> piecesOf(Colour turn);
  bool getTurn();
  bool onSameLine(int from, int to, int direction);
  void newPin(int pinnedPiece, int kingIndex, int attackerIndex, int direction);

  // other private functions:
  std::vector<MoveType>& slidingMoves(PieceData p, int index,
                    const std::array<PieceData, 64>& currentBoard);
  std::vector<MoveType>& knightMoves(PieceData p, int index,
                   const std::array<PieceData, 64>& currentBoard);
  std::vector<MoveType>& pawnMoves(PieceData p, int index,
                 const std::array<PieceData, 64>& currentBoard);
  bool enPassantLegalityCheck(
      PieceData p, std::vector<MoveType>& moves,
      int newIndex);  // handles the special case where taking an en passant
                      // would be an illegal move, due to a rook/queen lasering
                      // through the two pawns to the king
  int pseudoLegalKingMoves(
      PieceData p, std::array<PieceData, 64>& moves,
      const std::array<PieceData, 64>&
          currentBoard);  // outputs the 8 moves around king, and castling moves
                          // if legal. doesn't check if move is legal.
  std::vector<MoveType>& kingMoves(std::vector<MoveType>&
                     legalKingMoves);  // checks if each king move would put the
                                       // king in check (and castle stuff), if
                                       // so then its filtered out
  void findAttackedSquares(const std::array<PieceData, 64>& chessBoard,
                           std::vector<MoveType>& attackingMoves, Colour colour);
  bool isInCheck();
  void findAttackersOnKing(const std::vector<MoveType>& attackingMoves,
                           bool& multipleAttackers,
                           std::optional<MoveType>& checkingMove);
  void restrictMoves(std::optional<MoveType> checkingMove);

  void attacks(const std::array<PieceData, 64>& chessBoard,
               std::vector<MoveType>& attackingMoves, int colour, int index);
  void pawnAttacks(std::vector<MoveType>& attackingMoves, int colour,
                   int index);
  void knightAttacks(std::vector<MoveType>& attackingMoves, int index);
  void bishopAttacks(const std::array<int, 64>& chessBoard,
                     std::vector<MoveType>& attackingMoves, int index);
  void rookAttacks(const std::array<int, 64>& chessBoard,
                   std::vector<MoveType>& attackingMoves, int index);
  void queenAttacks(const std::array<int, 64>& chessBoard,
                    std::vector<MoveType>& attackingMoves, int index);
  void kingAttacks(std::vector<MoveType>& attackingMoves, int index);

 public:
  Board();  // constructor which sets the board to starting position
  void takePiece(int index);  // used only for en passant moves
  void promotePawn(int prevIndex, int newIndex, PieceData piece);
  PieceData getPiece(int index) const;
  std::vector<MoveType> legalMoves(int index, PieceData piece,
                                   std::array<PieceData, 64> currentBoard);
  void resetArrays();
  void generateAllMoves(Colour turn);
  std::vector<MoveType> checkMove(int index);
  void findPinsToKing(Colour turn);
  void isPinned(int index, PieceData p, std::vector<MoveType>& moves);
  int canTake(int index, Colour colour, const std::array<PieceData, 64>& currentBoard)
      const;  // index is for the piece we want to know if we can take, colour
              // is of the piece which is moving
  void setEnPassantFile(int column);
  void updateBoard(int prevIndex, int newIndex);
  void canCastle();
  void resetEnPassant();
  void findCheckingMoves();
  void deleteNonBlockingMoves(std::vector<MoveType>& moves);
  void findKing();

  // debugging functions:
  void printBoard();
};

#endif  // BOARD_H
