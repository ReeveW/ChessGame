#include "Game.h"
#include "Board.h"

#include <SFML/Graphics.hpp>
#include <SFML/Window/Event.hpp>
#include <array>
#include <iostream>
#include <string>

Game::Game() : window(sf::VideoMode(800, 800), "Chess Game") {
  window.setFramerateLimit(120);
}

void Game::run() {
  summonStartingSprites();
  board.generateAllMoves();

  while (window.isOpen()) {
    sf::Event event{};
    while (window.pollEvent(event)) {
      if (event.type == sf::Event::Closed) {
        window.close();
      } else if (isPromoting) {
        choosePromotionPiece(event);
      } else {
        handleDragAndDrop(event);
      }
    }

    updateDragPosition();

    window.clear();
    for (auto& s : sprite) {
      if(draggedPiece && &s == draggedPiece){
        continue;
      }
      window.draw(s);
    }
    lightValidSquares(validMoves);
    if(draggedPiece){
      window.draw(*draggedPiece);
    }
    window.display();
  }
}

void Game::handleMouseClick(sf::Event& event) {
  sf::Vector2f mousePos =
      window.mapPixelToCoords({event.mouseButton.x, event.mouseButton.y});
  int col = static_cast<int>(mousePos.x) / 100;
  int row = static_cast<int>(mousePos.y) / 100;
  prevIndex = row * 8 + col;
  pieceId = board.getPiece(prevIndex);
  if (pieceId / 8 == turn) {
    validMoves = board.checkMove(prevIndex);
    for (int i = 1; i < sprite.size(); ++i) {
      if (sprite[i].getGlobalBounds().contains(mousePos)) {
        draggedPiece = &sprite[i];
        offset = mousePos - sprite[i].getPosition();
        isDragging = true;
        break;
      }
    }
  }
}

void Game::dropPiece() {
  sf::Vector2f pos = draggedPiece->getPosition();
  int col = static_cast<int>(pos.x + 50) / 100;
  int row = static_cast<int>(pos.y + 50) / 100;
  int newIndex = row * 8 + col;
  bool switchTurn = true;

  bool foundMove = false;
  for (const auto& move : validMoves) {
    if (move.to == newIndex) {
      foundMove = true;
      switch (move.typeOfMove) {
        case 2:
          takePiece(row, col);
          break;
        case 3:
          castling(row, col);
          break;
        case 4:
          takingEnPassant(row, col);
          break;
        case 5:
          isPromoting = true;
          promotingPieceIndex = newIndex;
          promotingPieceSprite = draggedPiece;
          draggedPiece->setPosition(col * 100.f, row * 100.f);
          sprite[33].setPosition(0, 0);
          switchTurn = false;
          break;
        default:
          if ((pieceId & 1) && abs(move.to - move.from) == 16) {
            board.setEnPassantFile(col);
          }
          break;
      }
      if (switchTurn) {
        draggedPiece->setPosition(col * 100.f, row * 100.f);
        board.updateBoard(prevIndex, newIndex);
        nextTurn();
        checkPawnMove(prevIndex, newIndex);
        break;
      }
    }
  }
  if (!foundMove) {
    draggedPiece->setPosition((prevIndex % 8) * 100.f, (prevIndex / 8) * 100.f);
  }
}

void Game::handleDragAndDrop(sf::Event& event) {
  if (event.type == sf::Event::MouseButtonPressed &&
      event.mouseButton.button == sf::Mouse::Left) {
    handleMouseClick(event);
  }

  if (event.type == sf::Event::MouseButtonReleased &&
      event.mouseButton.button == sf::Mouse::Left) {
    if (draggedPiece) {
      dropPiece();
    }
    draggedPiece = nullptr;
    isDragging = false;
    validMoves = {};
  }
}

void Game::takeNonPromotingPiece(int row, int col,
                                 sf::Sprite* promotingPieceSprite) {
  for (int i = 1; i < 33; ++i) {
    if (sprite[i].getGlobalBounds().contains(col * 100.f, row * 100.f) &&
        (&sprite[i] != promotingPieceSprite)) {
      sprite[i].setPosition(1000, 1000);
      pieceTaken = i;
    }
  }
}

void Game::undoPromotion(int row, int col) {
  promotingPieceSprite->setPosition((prevIndex % 8) * 100.f,
                                    (prevIndex / 8) * 100.f);

  sprite[33].setPosition(1000, 1000);
  isPromoting = false;
  promotingPieceSprite = nullptr;
  promotingPieceId = 0;
  promotingPieceIndex = -1;
  if (pieceTaken) {
    sprite[pieceTaken].setPosition(col * 100.f, row * 100.f);
  }
}

void Game::choosePromotionPiece(sf::Event& event) {
  if (!promotingPieceSprite) {
    throw std::runtime_error("sprite of the promoting piece is missing");
  }
  int col = promotingPieceIndex % 8;
  int row = promotingPieceIndex / 8;
  int colour = turn;

  if (board.getPiece(promotingPieceIndex)) {
    takeNonPromotingPiece(row, col, promotingPieceSprite);
  }

  if (event.type == sf::Event::MouseButtonPressed &&
      event.mouseButton.button == sf::Mouse::Left) {
    sf::Vector2f mousePos =
        window.mapPixelToCoords({event.mouseButton.x, event.mouseButton.y});
    if (mousePos.x > 400 || mousePos.y > 100) {
      undoPromotion(row, col);

      return;
    }
    if (mousePos.x < 100) {
      board.promotePawn(promotingPieceIndex, 5 + (colour * 8), prevIndex);
      promotingPieceSprite->setTexture(texture[5 + (colour * 6)]);

    } else if (mousePos.x < 200) {
      board.promotePawn(promotingPieceIndex, 4 + (colour * 8), prevIndex);
      promotingPieceSprite->setTexture(texture[4 + (colour * 6)]);

    } else if (mousePos.x < 300) {
      board.promotePawn(promotingPieceIndex, 3 + (colour * 8), prevIndex);
      promotingPieceSprite->setTexture(texture[3 + (colour * 6)]);

    } else if (mousePos.x < 400) {
      board.promotePawn(promotingPieceIndex, 2 + (colour * 8), prevIndex);
      promotingPieceSprite->setTexture(texture[2 + (colour * 6)]);
    }
    nextTurn();
    sprite[33].setPosition(1000, 1000);
    isPromoting = false;
    promotingPieceSprite = nullptr;
    promotingPieceId = 0;
    promotingPieceIndex = -1;
    pieceTaken = 0;
  }
}

void Game::checkPawnMove(int prevIndex, int newIndex) {
  int prevRow = prevIndex / 8;
  int newRow = newIndex / 8;
  bool isPawn =
      (board.getPiece(newIndex) == 1 || board.getPiece(newIndex) == 9);
  if (isPawn && abs(newRow - prevRow) == 2) {
    return;
  }
  board.resetEnPassant();
}

void Game::castling(int row, int col) {
  int side = col - 5;  // neg means queenside, pos is kingside

  if (side > 0) {
    board.updateBoard(row * 8 + 7, row * 8 + 5);
    float xcoord = 7 * 100.f;
    float ycoord = row * 100.f;
    for (int i = 1; i < sprite.size(); ++i) {
      if (sprite[i].getGlobalBounds().contains(xcoord, ycoord)) {
        sprite[i].setPosition(5 * 100.f, ycoord);
        break;
      }
    }
  } else {
    board.updateBoard(row * 8 + 0, row * 8 + 3);
    float xcoord = 0 * 100.f;
    float ycoord = row * 100.f;
    for (int i = 1; i < sprite.size(); ++i) {
      if (sprite[i].getGlobalBounds().contains(xcoord, ycoord)) {
        sprite[i].setPosition(3 * 100.f, ycoord);
        break;
      }
    }
  }
}

void Game::takingEnPassant(int row, int col) {
  ;
  int ROW_BEHIND_PIECE = (pieceId / 8) ? -1 : 1;
  for (int i = 1; i < sprite.size(); ++i) {
    if (sprite[i].getGlobalBounds().contains(
            col * 100.f, (row + ROW_BEHIND_PIECE) * 100.f) &&
        (&sprite[i] != draggedPiece)) {
      sprite[i].setPosition(1000, 1000);
    }
  }
  board.takePiece((row + ROW_BEHIND_PIECE) * 8 + col);
}

void Game::takePiece(int row, int col) {
  for (int i = 1; i < sprite.size(); ++i) {
    if (sprite[i].getGlobalBounds().contains(col * 100.f, row * 100.f) &&
        (&sprite[i] != draggedPiece)) {
      sprite[i].setPosition(1000, 1000);
    }
  }
}

void Game::lightValidSquares(std::vector<moveType>& moves) {
  for (auto move : moves) {
    float x = move.to % 8 * 100.f;
    float y = move.to / 8 * 100.f;
    sf::RectangleShape highlight(sf::Vector2f(100.f, 100.f));
    highlight.setFillColor(sf::Color(255, 0, 0, 50));
    highlight.setPosition(x, y);
    window.draw(highlight);
  }
}

void Game::updateDragPosition() {  // makes sprite follow mouse as you're
                                   // dragging it around
  if (isDragging && draggedPiece) {
    sf::Vector2f mousePos =
        window.mapPixelToCoords(sf::Mouse::getPosition(window));
    draggedPiece->setPosition(mousePos - offset);
  }
}

void Game::loadSprites() {
  std::array<std::string, 15> file = {"chessboard.jpg",
                                      "wp.png",
                                      "wr.png",
                                      "wn.png",
                                      "wb.png",
                                      "wq.png",
                                      "wk.png",
                                      "bp.png",
                                      "br.png",
                                      "bn.png",
                                      "bb.png",
                                      "bq.png",
                                      "bk.png",
                                      "promochoiceswhite.png",
                                      "promochoicesblack.png"};
  // array of the images i use as sprites
  int j = 0;
  for (int i = 0; i < texture.size();
       ++i) {  // loop that makes 1 texture of each file, and sprites equal to
               // the amount of pieces we need on the starting chessboard.
    std::string path = "images/" + file[i];
    if (!texture[i].loadFromFile(
            path)) {  // makes texture for each of the images, if missing will
                      // send a message
      std::cout << path << " is missing" << std::endl;
    }
    if (i == 1 || i == 7) {  // loop to make 8 pawn sprites for each side
      for (int k = 0; k < 8; ++k) {
        sprite[j].setTexture(texture[i]);
        j++;
      }
    } else if (i == 2 || i == 3 || i == 4 || i == 8 || i == 9 ||
               i == 10) {  // loop for rooks, knights and bishops to make two
                           // of each
      for (int k = 0; k < 2; ++k) {
        sprite[j].setTexture(texture[i]);
        j++;
      }
    } else {  // all others we just need 1 (chessboard, queens and kings)
      sprite[j].setTexture(texture[i]);
      j++;
    }
  }
}

void Game::deleteSprites() {}

void Game::nextTurn() {
  board.canCastle();

  turn = !turn;
  board.findKing();
  board.findCheckingMoves();
  board.findPinsToKing(!turn);
  board.generateAllMoves();
}

void Game::summonStartingSprites() {
  float scale = 100.0 / 333;
  int coordMult = 100;
  sprite[0].setScale(1 / 2.4, 1 / 2.4);
  for (int i = 1; i < sprite.size(); ++i) {
    sprite[i].setScale(scale,
                       scale);  // loop that sets all sprites to proper scale
  }

  int j = 1;
  for (int k = 0; k < 8; ++k) {
    sprite[j].setPosition(k * coordMult, 6 * coordMult);
    j++;
  }

  sprite[j].setPosition(0 * coordMult, 7 * coordMult);
  j++;
  sprite[j].setPosition(7 * coordMult, 7 * coordMult);
  j++;

  sprite[j].setPosition(1 * coordMult, 7 * coordMult);
  j++;
  sprite[j].setPosition(6 * coordMult, 7 * coordMult);
  j++;
  sprite[j].setPosition(2 * coordMult, 7 * coordMult);
  j++;
  sprite[j].setPosition(5 * coordMult, 7 * coordMult);
  j++;
  sprite[j].setPosition(3 * coordMult, 7 * coordMult);
  j++;
  sprite[j].setPosition(4 * coordMult, 7 * coordMult);
  j++;
  for (int k = 0; k < 8; ++k) {
    sprite[j].setPosition(k * coordMult, 1 * coordMult);
    j++;
  }
  sprite[j].setPosition(0 * coordMult, 0 * coordMult);
  j++;
  sprite[j].setPosition(7 * coordMult, 0 * coordMult);
  j++;
  sprite[j].setPosition(1 * coordMult, 0 * coordMult);
  j++;
  sprite[j].setPosition(6 * coordMult, 0 * coordMult);
  j++;
  sprite[j].setPosition(2 * coordMult, 0 * coordMult);
  j++;
  sprite[j].setPosition(5 * coordMult, 0 * coordMult);
  j++;
  sprite[j].setPosition(3 * coordMult, 0 * coordMult);
  j++;
  sprite[j].setPosition(4 * coordMult, 0 * coordMult);
  sprite[33].setPosition(1000, 1000);
  sprite[34].setPosition(1000, 1000);
}
