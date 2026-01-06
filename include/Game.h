
#ifndef GAME_H
#define GAME_H

#include <SFML/Graphics.hpp>
#include <SFML/Window/Event.hpp>
#include <array>

#include "Board.h"
#include "ChessTypes.h"

class Game {
 private:
  sf::RenderWindow window;
  std::array<sf::Texture, 15> texture;
  std::array<sf::Sprite, 35> sprite;
  sf::Sprite* draggedPiece = nullptr;  // pointer to the sprite being dragged
  sf::Vector2f offset;  // difference between mouse and sprite origin
  bool isDragging = false;
  Board board;
  std::vector<MoveType> validMoves;
  Colour _turn = Colour::White;
  int prevIndex;
  int pieceId;

  // stuff for pawn promotion
  bool isPromoting = false;
  int promotingPieceId = 0;
  int promotingPieceIndex;
  sf::Sprite* promotingPieceSprite = nullptr;
  int pieceTaken = 0;
  //

 public:
  Game();
  void run();
  void loadSprites();
  void deleteSprites();
  void summonStartingSprites();
  void movePiece();
  void choosePromotionPiece(sf::Event& event);
  void checkPawnMove(int prevIndex, int newIndex);
  void handleDragAndDrop(sf::Event& Event);
  void updateDragPosition();
  void lightValidSquares(std::vector<MoveType>& moves);
  void takePiece(int row, int col);
  void castling(int row, int col);
  void takingEnPassant(int row, int col);
  void nextTurn();
  void dropPiece(sf::Sprite* draggedPiece);
  sf::Sprite* findPiece(sf::Event& event);
};

#endif  // GAME_H
