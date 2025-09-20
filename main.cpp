

#include <SFML/Graphics.hpp>
#include <iostream>

#include "Game.h"

int main() {
    Game game;
    game.loadSprites();
    game.run();

    return 0;
}
