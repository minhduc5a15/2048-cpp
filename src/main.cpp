#include "game/game.h"

/**
 * @brief The entry point for the console version of the 2048 game.
 *
 * This function creates an instance of the `Game` class and calls its `run` method
 * to start the main game loop.
 */
int main() {
    tfe::game::Game game;
    game.run();
    return 0;
}