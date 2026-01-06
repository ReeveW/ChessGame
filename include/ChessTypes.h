#pragma once

#include <vector>


enum class Piece { Empty, Pawn, Knight, Bishop, Rook, Queen, King };
enum class Colour { Empty, White, Black };
struct PieceData {
  Piece piece;
  Colour colour;
};

// outlines all of the standard chess pieces
constexpr PieceData E{Piece::Empty, Colour::Empty};

constexpr PieceData WP{Piece::Pawn, Colour::White};
constexpr PieceData WN{Piece::Knight, Colour::White};
constexpr PieceData WB{Piece::Bishop, Colour::White};
constexpr PieceData WR{Piece::Rook, Colour::White};
constexpr PieceData WQ{Piece::Queen, Colour::White};
constexpr PieceData WK{Piece::King, Colour::White};

constexpr PieceData BP{Piece::Pawn, Colour::Black};
constexpr PieceData BN{Piece::Knight, Colour::Black};
constexpr PieceData BB{Piece::Bishop, Colour::Black};
constexpr PieceData BR{Piece::Rook, Colour::Black};
constexpr PieceData BQ{Piece::Queen, Colour::Black};
constexpr PieceData BK{Piece::King, Colour::Black};

// for the standard chess board with all pieces in their starting position
constexpr std::array<PieceData, 64> defaultBoard = {
  BR, BN, BB, BQ, BK, BB, BN, BR,
  BP, BP, BP, BP, BP, BP, BP, BP,

  E, E, E, E, E, E, E, E,
  E, E, E, E, E, E, E, E,
  E, E, E, E, E, E, E, E,
  E, E, E, E, E, E, E, E,

  WP, WP, WP, WP, WP, WP, WP, WP,
  WR, WN, WB, WQ, WK, WB, WN, WR
};


enum class MoveCategories {
  Regular,
  Capture,
  Defends,
  SeeThroughKing,
  Castling,
  DoublePawnStep,
  EnPassant,
  Promotion,
  PromotionCapture,
  Missing
};

struct MoveType {
  int from;
  int to;
  MoveCategories typeOfMove;
};
struct PieceMoves {
  int index;
  std::vector<MoveType> moves;
};
