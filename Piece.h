//
// Created by johnathan-porkonimus on 6/23/25.
//

#ifndef PIECE_H
#define PIECE_H

class Piece {
private:
    int id; // pieces are identified through a number id. in binary, the first three digits represent the type of piece (0-7), and the next digit represents the colour.
            // empty = 0, pawn = 1, rook = 2, knight = 3, bishop = 4, queen = 5, king = 6
            // white = 0, black = 8
public:
    Piece();
    explicit Piece(int id);
    int getId() const;
    int getType() const;
    bool isWhite() const;
    bool isEmpty() const;
};


#endif //PIECE_H
