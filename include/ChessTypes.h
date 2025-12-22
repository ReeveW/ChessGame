enum class Piece { Empty, Pawn, Knight, Bishop, Rook, Queen, King };
enum class Colour { White, Black };
struct PieceData {
  Piece piece;
  Colour colour;
  int index;
};

enum class MoveCategory {
  Regular,
  Capture,
  Castling,
  DoublePawnStep,
  EnPassant,
  Promotion,
  PromotionCapture
};
