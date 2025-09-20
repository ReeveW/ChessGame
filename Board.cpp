

#include "Game.h"
#include <iostream>
#include "Board.h"
#include <any>
#include <functional>
#include <vector>
#include <unordered_set>
#include <optional>


std::array<Point, 8> sliders = {
    Point{0, -1},
    {1, -1},
    {1, 0},
    {1, 1},
    {0, 1},
    {-1, 1},
    {-1, 0},
    {-1, -1},

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
    * Q is for queen, 0-7 denotes the possible squares she can move, in the order they're stored in the translations array.
    * the multiples of the translations are also shown.
    * from this table you can extrapolate what rook moves would be (just even numbers), bishop moves (odd numbers) and king moves (no multiples)
    */

};


std::array<Point, 8> knightDirections = {
    Point{1, -2},
    {2, -1},
    {2, 1},
    {1, 2},
    {-1, 2},
    {-2, 1},
    {-2, -1},
    {-1, -2},

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
    * N is for knight, 0-7 denotes the possible moves in the order they're stored in the array
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
//      * P is for pawn, 0-4 denotes possible moves in the order they're stored in the array.
//      * 1 is only available if the pawn hasn't moved, 2 and 3 are only possible when taking a piece or for en passant.
//      * All Point values are multiplied by -1 if the pawn is black, as it moves the opposite way.
//      */
// };

std::vector<int> Board::piecesOf(int turn) {
    std::vector<int> pieceLocations;
    forEachSquare([&] (int i) {
        if (board[i] / 8 == !turn && board[i] != 0) {
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
    int toRow   = to / 8,   toCol   = to % 8;

    switch (direction) {
        case 1: case -1: return fromRow == toRow;
        case 8: case -8: return fromCol == toCol;
        case 9: case -9: return (fromRow - fromCol) == (toRow - toCol);
        case 7: case -7: return (fromRow + fromCol) == (toRow + toCol);
    }
    return false;
}

void Board::newPin(int pinnedPiece, int kingIndex, int attackerIndex, int direction) {
    pinInfo pin;
    pin.pinIndex = pinnedPiece;
    int start = kingIndex + direction;
    // Record path between pinned piece and king
    for (int currentSquare = start; currentSquare != attackerIndex + direction; currentSquare += direction) { // loop goes from 1 step past king to attackerIndex
        pin.pathToKing.insert(currentSquare);
    }
    pins.push_back(pin);
}

void Board::slidingMoves(pieceData p, std::vector<moveType>& moves, const std::array<int, 64>& currentBoard) {
    std::vector<int> directions;

    if (p.type == 2) {
        directions = {0, 2, 4, 6};  // rook directions
    }
    else if (p.type == 4) {
        directions = {1, 3, 5, 7};  // bishop directions
    }
    else if (p.type == 5) {
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
                    moveType take {p.index, i, 2};
                    moves.push_back(take);
                }
                break;
            }
            moveType slide {p.index, i, 1};
            moves.push_back(slide);
        }
    }
}

void Board::knightMoves(pieceData p, std::vector<moveType>& moves, const std::array<int, 64>& currentBoard) {
    for (auto nm : knightDirections) {
        int x = (p.index % 8) + nm.X;
        int y = (p.index / 8) + nm.Y;
        if (x < 0 || x > 7 || y < 0 || y > 7) {
            continue;
        }

        int i = x + (8 * y);

        if (currentBoard[i] && canTake(i, p.isBlack, currentBoard)) {
            moveType take {p.index, i, 2};
            moves.push_back(take);
        }else if (!currentBoard[i]){
            moveType horse {p.index, i, 1};
            moves.push_back(horse);
        }
    }
}

void Board::pawnMoves(pieceData p, std::vector<moveType>& moves, const std::array<int, 64>& currentBoard) {
    const int ADD_ROW = p.isBlack ? 8 : -8;
    int row = p.index / 8;
    int col = p.index % 8;
    ;

    int forwardOne = p.index + ADD_ROW;
    int forwardTwo = p.index + 2 * ADD_ROW;


    if (!currentBoard[forwardOne]) {
        moveType f1 {};
        f1.from = p.index;
        f1.to = forwardOne;
        if (p.isBlack && row == 6 || !p.isBlack && row == 1) {
            f1.typeOfMove = 5; // means pawn is promoting
        }else {
            f1.typeOfMove = 1;
        }
        moves.push_back(f1);
        // Double move from starting position
        if ((p.isBlack && row == 1) || (!p.isBlack && row == 6)) {
            if (!currentBoard[forwardTwo]) {
                moveType f2{p.index, forwardTwo, 1};
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
                if (currentBoard[captureIndex] && canTake(captureIndex, p.isBlack, currentBoard)) {
                    moveType capture {};
                    capture.from = p.index;
                    capture.to = captureIndex;
                    if (!p.isBlack && row == 1 || p.isBlack && row == 6) {
                        capture.typeOfMove = 5;
                    }else {
                        capture.typeOfMove = 2;
                    }
                    moves.push_back(capture);
                }
                // En passant capture
                if (enPassantFile == newCol && row == 3 + p.isBlack) {
                    if (enPassantLegalityCheck(p, moves, captureIndex)) {
                        moveType enPassant {p.index, captureIndex, 4};
                        moves.push_back(enPassant);
                    }
                }
            }
        }
    }
}

bool Board::enPassantLegalityCheck(pieceData p, std::vector<moveType> &moves, int newIndex) {
    int directionToTakenPawn = 8;
    if (p.isBlack) {
        directionToTakenPawn = -8;
    }

    // simulating the en passant capture
    auto newBoard = board;
    newBoard[newIndex] = 1 + p.isBlack * 8;
    newBoard[p.index] = 0;
    newBoard[newIndex + directionToTakenPawn] = 0;



    std::vector<moveType> newSquares;
    std::vector<moveType> attackingMoves;


    findAttackedSquares(newBoard, attackingMoves, p.isBlack * 8, !p.isBlack * 8);


    // to debug

    bool isLegal = !isInCheck();

    return isLegal;
}


int Board::pseudoLegalKingMoves(pieceData p, std::array<int, 64>& moves, const std::array<int, 64>& currentBoard) { // ignore moves that would put king in check for now
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
        }else if (!currentBoard[i]){
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
    }else {
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

void Board::kingMoves(std::vector<moveType>& legalKingMoves) {  // king can take pieces that are defended, and if lasered can move to the square behind the laser still
    //step 3: find all possible squares the king can move to, if it can castle then check the squares in between as well
    std::array <int, 64> legalSquares = {};
    int castleSides = pseudoLegalKingMoves(king, legalSquares, board);
    forEachSquare([&] (int i) {
        if (legalSquares[i] && squaresBeingAttacked[i]) {
            legalSquares[i] = 0;
        }
    });

    // queenside
    if ((castleSides & 1) && // single ampersand is to grab x (this case being 1) bit and see if its true or not
        (squaresBeingAttacked[3 + (56 * !king.isBlack)] || squaresBeingAttacked[2 + (56 * !king.isBlack)] || squaresBeingAttacked[4 + (56 * !king.isBlack)] || inCheck)) {
        legalSquares[2 + (56 * !king.isBlack)] = 0;
    }

    // king side
    if ((castleSides & 2) &&
        (squaresBeingAttacked[5 + (56 * !king.isBlack)] || squaresBeingAttacked[6 + (56 * !king.isBlack)] || inCheck)) {
        legalSquares[6 + (56 * !king.isBlack)] = 0;
    }


    forEachSquare([&] (int i) {
        if (legalSquares[i]) {
            moveType kingMove {king.index, i, legalSquares[i]};
            legalKingMoves.push_back(kingMove);
        }
    });


    //print out stuff to test

    // std::cout << "turn: " << turn << std::endl;
    // std::cout << "king index: " << king.index << std::endl;
    // std::cout << "is in check: " << inCheck << std::endl;
    // std::cout << "castle sides: " << castleSides << std::endl;


    // std::cout << "squares being attacked: " << std::endl;
    // for (int i = 0; i < 64; ++i) {
    //     if (i % 8 == 0 && i != 0) {
    //         std::cout << std::endl;
    //     }
    //     std::cout << squaresBeingAttacked[i] << " ";
    // }
    // std::cout << std::endl;
    // //
    // std::cout << "legalSquares: " << std::endl;
    // for (int i = 0; i < 64; ++i) {
    //     if (i % 8 == 0 && i != 0) {
    //         std::cout << std::endl;
    //     }
    //     std::cout << legalSquares[i] << " ";
    // }
    //
    // std::cout << std::endl << std::endl;
}


void Board::findAttackedSquares(const std::array<int, 64>& chessBoard, std::vector<moveType>& attackingMoves, int colour, int oppositeColour) {
    squaresBeingAttacked = {};
    forEachSquare([&] (int i) {
        if ((chessBoard[i] / 8) != colour) {
            attacks(chessBoard, attackingMoves, oppositeColour, i);
        }
    });

    // if (chessBoard[i] == 1 + oppositeColour) {
    //     int row = 8;
    //     if (colour) {
    //         row = -8;
    //     }
    //     if (i % 8 != 0) {
    //         squaresBeingAttacked[i - 1 + row] = 1;
    //     }
    //     if (i % 8 != 7) {
    //         squaresBeingAttacked[i + 1 + row] = 1;
    //     }
    // }else if (chessBoard[i] == 6 + oppositeColour) {
    //     pieceData enemyKing {};
    //     enemyKing.isBlack = chessBoard[i] / 8;
    //     enemyKing.type = 6;
    //     enemyKing.index = i;
    //     std::array<int, 64> kingArray = {};
    //     pseudoLegalKingMoves(enemyKing, kingArray, chessBoard);
    //     forEachSquare([&] (int j) {
    //         if (kingArray[j]) {
    //             squaresBeingAttacked[j] = 1;
    //             moveType kingAttack {i, j, 1};
    //             attackingMoves.push_back(kingAttack);
    //         }
    //     });
    // }
    // else if (chessBoard[i] != 6 + oppositeColour){ // was recursively calling this function on each player's kings, leading to an infinite loop. need to call pseudo legal king moves instead
    //     newSquares = legalMoves(i, chessBoard[i], chessBoard);
    //     for (auto square : newSquares) {
    //         squaresBeingAttacked[square.to] = 1;
    //         attackingMoves.push_back(square);
    //     }
    //     newSquares.clear();
    // }
}

bool Board::isInCheck() { // this function is being called after all the moves have been generated i think?
    if (squaresBeingAttacked[king.index]) {
        return true;
    }
    return false;
}

void Board::findAttackersOnKing(const std::vector<moveType> &attackingMoves, bool &multipleAttackers, std::optional<moveType> &checkingMove) {
    multipleAttackers = false;
    for (moveType move : attackingMoves) {
        if (move.to == king.index) {
            if (checkingMove) {
                multipleAttackers = true;
            }else {
                checkingMove = {move};
            }
        }
    }
}

void Board::restrictMoves(std::optional<moveType> checkingMove) { // blocking checks doesn't work properly anymore
    int checkerIndex = checkingMove -> from;
    int id = board[checkerIndex] / 8;
    blockingSquares.insert(checkerIndex); // attacker can always just be taken by a piece
    if (id == 1 || id == 3) {
        //disallow all piece moves except king and taking the checker
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
    }else if (kingRow == checkerRow) {
        rowDirection = 0;
    }else {
        rowDirection = 1;
    }
    if (kingCol > checkerCol) {
        colDirection = -1;
    }else if (kingCol == checkerCol) {
        colDirection = 0;
    }else {
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

void Board::attacks(const std::array<int, 64>& chessBoard, std::vector<moveType>& attackingMoves, int colour, int index) {
    int type = chessBoard[index] - colour;
    if (type == 1) {
        pawnAttacks(attackingMoves, colour, index);
    }else if (type == 2) {
        rookAttacks(chessBoard, attackingMoves, index);
    }else if (type == 3) {
        knightAttacks(attackingMoves, index);
    }else if (type == 4) {
        bishopAttacks(chessBoard, attackingMoves, index);
    }else if (type == 5) {
        queenAttacks(chessBoard, attackingMoves, index);
    }else if (type == 6) {
        kingAttacks(attackingMoves, index);
    }
}


void Board::pawnAttacks(std::vector<moveType>& attackingMoves, int colour, int index) {
    int row = -8;
    if (colour) {
        row = 8;
    }
    if (index % 8 != 0) {
        squaresBeingAttacked[index - 1 + row] = 1;
        moveType pawnAttack {index, index - 1 + row, 1};
        attackingMoves.push_back(pawnAttack);
    }
    if (index % 8 != 7) {
        squaresBeingAttacked[index + 1 + row] = 1;
        moveType pawnAttack {index, index + 1 + row, 1};
        attackingMoves.push_back(pawnAttack);
    }
}
void Board::knightAttacks(std::vector<moveType>& attackingMoves, int index) {
    for (Point kd : knightDirections) {
        int row = index / 8 + kd.Y;
        int col = index % 8 + kd.X;

        if (row >= 0 && row < 8 && col >= 0 && col < 8) { // if nothing is out of bounds
            int move = row * 8 + col;
            squaresBeingAttacked[move] = 1;
            moveType knightAttack {index, move, 1};
            attackingMoves.push_back(knightAttack);
        }
    }
}

void Board::bishopAttacks(const std::array<int, 64>& chessBoard, std::vector<moveType>& attackingMoves, int index) {
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
                moveType take {index, i, 1}; // basically the same as sliderMoves except we mark attacks regardless of the piece on the square
                attackingMoves.push_back(take);
                break;
            }
            squaresBeingAttacked[i] = 1;
            moveType slide {index, i, 1};
            attackingMoves.push_back(slide);
        }
    }
}

void Board::rookAttacks(const std::array<int, 64>& chessBoard, std::vector<moveType>& attackingMoves, int index) {
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
                moveType take {index, i, 1}; // basically the same as sliderMoves except we mark attacks regardless of the piece on the square
                attackingMoves.push_back(take);
                break;
            }
            squaresBeingAttacked[i] = 1;
            moveType slide {index, i, 1};
            attackingMoves.push_back(slide);
        }
    }
}

void Board::queenAttacks(const std::array<int, 64>& chessBoard, std::vector<moveType>& attackingMoves, int index) {
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
                moveType take {index, i, 1}; // basically the same as sliderMoves except we mark attacks regardless of the piece on the square
                attackingMoves.push_back(take);
                break;
            }
            squaresBeingAttacked[i] = 1;
            moveType slide {index, i, 1};
            attackingMoves.push_back(slide);
        }
    }
}

void Board::kingAttacks(std::vector<moveType>& attackingMoves, int index) {
    for (auto move : sliders) {

        int row = index / 8 + move.Y;
        int col = index % 8 + move.X;

        if (row >= 0 && row < 8 && col >= 0 && col < 8) {
            // if nothing is out of bounds
            int to = row * 8 + col;squaresBeingAttacked[to] = 1;
            moveType kingMove {index, to, 1};
            attackingMoves.push_back(kingMove);
        }
    }
}

Board::Board(): board(), castleRights(), enPassantSquares() {

    // combine this into one big for loop from 0-63
    for (int i = 16; i < 48; ++i) {
        board[i] = 0;
    }
    for (int i = 0; i < 8; ++i) { // sets board value for white pawns
        board[i + 48] = 1;
    }
    for (int i = 0; i <= 4; ++i) { // sets board value for white queenside rook through king
        board[i + 56] = i + 2;
    }
    for (int i = 5; i < 8; ++i) { // sets board value for white kingside bishop through rook
        board[i + 56] = (9 - i);
    }


    for (int i = 0; i < 8; ++i) { // sets board value for black pawns
        board[i + 8] = 9;
    }
    for (int i = 0; i <= 4; ++i) { // sets board value for black queenside rook through king
        board[i] = i + 10;
    }
    for (int i = 5; i < 8; ++i) { // sets board value for black bishop through rook
        board[i] = (17 - i);
    }

    for (bool & enPassantSquare : enPassantSquares) {
        enPassantSquare = false;
    }


    for (bool & castleRight : castleRights) {
        castleRight = true;
    }

}

void Board::takePiece(int index) {
    board[index] = 0;
}

void Board::promotePawn(int index, int newId, int prevIndex) {
    board[prevIndex] = 0;
    board[index] = newId;
    lastPieceMoved = newId;
}

int Board::getPiece(int index) const {
    return board[index];
}

std::vector<moveType> Board::legalMoves(int index, int id, std::array<int, 64> currentBoard) {

    std::vector<moveType> moves; // vector where 0 is squares the piece cant move, 1 is legal moves, 2 is legal moves where it takes a piece, 3 is for castling, 4 for en passant, 5 for promotion squares
    pieceData p {};
    p.isBlack = id / 8;
    p.type = id % 8;
    p.index = index;


    if (p.type == 5 || p.type == 2 || p.type == 4) { // this handles the logic for the rooks, queens and bishops.
        slidingMoves(p, moves, currentBoard);
    }
    if (p.type == 3) {
        knightMoves(p, moves, currentBoard);
    }
    if (p.type == 1) {
        pawnMoves(p, moves, currentBoard);
    }
    if (p.type == 6) {
        kingMoves(moves);
    }

    isPinned(p, moves);

    if (inCheck) {
        if (p.type != 6) {
            deleteNonBlockingMoves(moves);
        }
    }

    return moves;
}

void Board::generateAllMoves() {
    bool turn = getTurn();

    allLegalMoves.clear();
    blockingSquares.clear();
    for (int piece : piecesOf(turn)) {
        pieceMoves currentPiece;
        currentPiece.index = piece;
        currentPiece.moves = legalMoves(piece, board[piece], board);
        if (!currentPiece.moves.empty()) {
            allLegalMoves.push_back(currentPiece);
        }
    }
    if (allLegalMoves.empty()) {
        if (inCheck) {
            std::cout << std::endl << "Checkmate!" << std::endl;
        }else {
            std::cout << std::endl << "Stalemate!" << std::endl;
        }
    }
}

std::vector<moveType> Board::checkMove(int index) {
    for (const pieceMoves& piece : allLegalMoves) {
        if (piece.index == index) {
            return piece.moves;
        }
    }
    pieceMoves empty;
    return {};
}

void Board::findPinsToKing(int turn) {
    pins.clear();

    int kingIndex = -1;
    forEachSquare([&](int i) {
        if (board[i] == 6 + turn * 8) {
            kingIndex = i;
        }
    });

    std::array<int, 8> directions = { 1, -1, 8, -8, 9, -9, 7, -7 };
    for (int direction : directions) {
        int current = kingIndex + direction;
        int pinnedPiece = -1;

        while (current >= 0 && current < 64 && onSameLine(kingIndex, current, direction)) {
            if (board[current] == 0) {
                current += direction;
                continue;
            }

            if (board[current] / 8 == turn) {
                //piece of same colour
                if (pinnedPiece == -1) {
                    pinnedPiece = current;
                    current += direction;
                } else {
                    break;
                }
            } else {
                //enemy piece
                int type = board[current] % 8;
                if ((direction == 1 || direction == -1 || direction == 8 || direction == -8) && (type == 2 || type == 5)) {
                    // same file (rook, queen)
                    if (pinnedPiece != -1) newPin(pinnedPiece, kingIndex, current, direction);
                } else if ((direction == 9 || direction == -9 || direction == 7 || direction == -7) && (type == 4 || type == 5)) {
                    // same diagonal(bishop, queen)
                    if (pinnedPiece != -1) newPin(pinnedPiece, kingIndex, current, direction);
                }
                break;
            }
        }
    }
}

void Board::isPinned(pieceData p, std::vector<moveType>& moves) {
    for (pinInfo& pin : pins) {
        if (pin.pinIndex == p.index) {
            auto newMoves = moves;
            moves.clear();
            //we copy moves to newMoves, then clear moves so that we can fill it with only pinned moves.
            for (moveType pieceSteps : newMoves) {
                if (pin.pathToKing.count(pieceSteps.to)) { // this is checking if the current move (pieceSteps.to) of the piece we're looking at (p) is in the stack containing the moves of the pin.
                    moves.push_back(pieceSteps);
                }
            }
        }
    }
}

int Board::canTake(int index, bool colour, const std::array<int, 64>& currentBoard) const {
    if (colour == currentBoard[index] / 8) { // this checks if the two pieces are the same colour, if so then it's simply an invalid move, otherwise it can be taken
        return 0;
    }
    return 2;
}

void Board::setEnPassantFile(int column) {
    enPassantFile = column;
}


void Board::updateBoard(int prevIndex, int newIndex) {

    int temp = board[prevIndex];
    board[prevIndex] = 0;
    board[newIndex] = temp;
    lastPieceMoved = board[newIndex];
    // std::cout << temp << " " << prevIndex << std::endl;
}

void Board::canCastle() {

    castleRights[0] = (castleRights[0] && (getPiece(4) == 14) && getPiece(0) == 10);

    castleRights[1] = (castleRights[1] && getPiece(4) == 14 && getPiece(7) == 10);

    castleRights[2] = (castleRights[2] && getPiece(60) == 6 && getPiece(56) == 2);

    castleRights[3] = (castleRights[3] && getPiece(60) == 6 && getPiece(63) == 2);
}

// resets the opposing players side of the en passant square array at the end of each players turn.
void Board::resetEnPassant() {
    enPassantFile = -1;
}



void Board::findCheckingMoves() {
    bool turn = getTurn();
    int colour = !turn * 8; // 8 when black, 0 when white
    int oppositeColour = turn * 8;
    bool multipleAttackers = false;

    std::vector<moveType> attackingMoves;
    attackingMoves.reserve(64);

    findAttackedSquares(board, attackingMoves, colour, oppositeColour);

    inCheck = isInCheck(); // pass moveType of attacker, and multipleAttackers
    if (inCheck) {
        std::optional<moveType> checkingMove;
        findAttackersOnKing(attackingMoves, multipleAttackers, checkingMove);
        if (multipleAttackers) {
            onlyKingToMove = true;
        }else {
            restrictMoves(checkingMove);

        }
    }
}

void Board::findKing() {
    bool turn = getTurn();
    forEachSquare([&] (int i) {
        if (board[i] == 6 + !turn * 8) {
            king.index = i;
            king.isBlack = !turn;
            king.type = 6;
        }
    });
}

void Board::deleteNonBlockingMoves(std::vector<moveType>& moves) {
    auto tempMoves = moves;
    moves.clear();
    for (const auto& move : tempMoves) {
        if (blockingSquares.find(move.to) != blockingSquares.end()) {
            moves.push_back(move);
        }
    }
}


void Board::printBoard() {
    forEachSquare([&] (int i) {
        std::cout << board[i] << " ";
        if ((i + 1) % 8 == 0) {
            std::cout << std::endl;
        }
    });
}





