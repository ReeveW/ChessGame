//
// Created by johnathan-porkonimus on 6/23/25.
//

#ifndef BOARD_H
#define BOARD_H

#include <array>
#include <functional>
#include <unordered_set>
#include "Piece.h"
#include <vector>
#include <optional>

struct Point { // struct to store coordinates on the board array
    int X, Y; // all Y values get multiplied by 8 when converted to board indices
};
struct pieceData{
    bool isBlack; // 0 is white, 1 is black
    int type;
    int index;
};
struct moveType {
    int from;
    int to;
    int typeOfMove;
};
struct pieceMoves {
    int index;
    std::vector<moveType> moves;
};
struct pinInfo {
    int pinIndex;
    std::unordered_set<int> pathToKing; // vector of indices which track the path between the pinner and the king
};


class Board {
private:
    std::array<int, 64> board;
    std::array<bool, 4> castleRights; // index 0 is black queenside, 1 is black kingside, 2 is white queenside, 3 is white kingside
    std::array<bool, 16> enPassantSquares; // array corresponding to the squares that are en passant-able. the square behind the pawn being taken is the one that
                                           // is switched to true, as this matches FEN notation.
    // bool turn; // white's turn if false, black's if true
    int enPassantFile = -1;
    int lastPieceMoved = 0;
    bool onlyKingToMove = false;
    bool inCheck = false;
    pieceData king {false, 6, 60};
    std::vector <pieceMoves> allLegalMoves;
    std::vector <pinInfo> pins;
    std::unordered_set<int> blockingSquares;
    std::array<int, 64> squaresBeingAttacked = {};


    // helper functions

    inline void forEachSquare(const std::function<void(int)>& action) { // used a lot to loop through each square in the board
        for (int i = 0; i < 64; ++i) {
            action(i);
        }
    }
    std::vector<int> piecesOf(int turn);
    bool getTurn();
    bool onSameLine(int from, int to, int direction);
    void newPin(int pinnedPiece, int kingIndex, int attackerIndex, int direction);


    // other private functions:
    void slidingMoves(pieceData p, std::vector<moveType>& moves, const std::array<int, 64>& currentBoard);
    void knightMoves(pieceData p, std::vector<moveType>& moves, const std::array<int, 64>& currentBoard);
    void pawnMoves(pieceData p, std::vector<moveType>& moves, const std::array<int, 64>& currentBoard);
    bool enPassantLegalityCheck(pieceData p, std::vector<moveType>& moves, int newIndex); // handles the special case where taking an en passant would be an illegal move, due to a rook/queen lasering through the two pawns to the king
    int pseudoLegalKingMoves(pieceData p, std::array<int, 64>& moves, const std::array<int, 64>& currentBoard); // outputs the 8 moves around king, and castling moves if legal. doesn't check if move is legal.
    void kingMoves(std::vector<moveType>& legalKingMoves); // checks if each king move would put the king in check (and castle stuff), if so then its filtered out
    void findAttackedSquares(const std::array<int, 64>& chessBoard, std::vector<moveType>& attackingMoves, int colour, int oppositeColour);
    bool isInCheck();
    void findAttackersOnKing(const std::vector<moveType>& attackingMoves, bool &multipleAttackers, std::optional<moveType> &checkingMove);
    void restrictMoves(std::optional<moveType> checkingMove);

    //attacks, these functions are different from the moves functions cause
    void attacks(const std::array<int, 64>& chessBoard, std::vector<moveType>& attackingMoves, int colour, int index);
    void pawnAttacks(std::vector<moveType>& attackingMoves, int colour, int index);
    void knightAttacks(std::vector<moveType>& attackingMoves, int index);
    void bishopAttacks(const std::array<int, 64>& chessBoard, std::vector<moveType>& attackingMoves, int index);
    void rookAttacks(const std::array<int, 64>& chessBoard, std::vector<moveType>& attackingMoves, int index);
    void queenAttacks(const std::array<int, 64>& chessBoard, std::vector<moveType>& attackingMoves, int index);
    void kingAttacks(std::vector<moveType>& attackingMoves, int index);


public:
    Board(); // constructor which sets the board to starting position
    void takePiece(int index); // used only for en passant moves
    void promotePawn(int index, int newId, int prevIndex);
    int getPiece(int index) const;
    std::vector<moveType> legalMoves(int index, int id, std::array<int, 64> currentBoard);
    void generateAllMoves();
    std::vector<moveType> checkMove(int index);
    void findPinsToKing(int turn);
    void isPinned(pieceData p, std::vector<moveType>& moves);
    int canTake(int index, bool colour, const std::array<int, 64>& currentBoard) const; // index is for the piece we want to know if we can take, colour is of the piece which is moving
    void setEnPassantFile(int column);
    void updateBoard(int prevIndex, int newIndex);
    void canCastle();
    void resetEnPassant();
    void findCheckingMoves();
    void deleteNonBlockingMoves(std::vector<moveType>& moves);
    void findKing();



    //debugging functions:
    void printBoard();
};

#endif //BOARD_H
