/*
 * Step 1: get all black and white pieces imported to SFML
 * Step 2: make classes for the pieces and the board
 * make the pieces interactable, so you can drag and drop pieces
 * integrate FEN notation
 */

#include <SFML/Graphics.hpp>
#include <iostream>

#include "Game.h"

int main() {
    Game game;
    game.loadSprites();
    game.run();

    return 0;
}
