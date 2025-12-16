

#include <SFML/Graphics.hpp>
#include <array>
#include <iostream>
#include <string>

#include "Game.h"

int main() {
  Game game;
  game.loadSprites();
  game.run();

  return 0;
}
