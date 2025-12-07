#include "gui/gui-game.h"

/**
 * @brief The entry point for the GUI (Raylib) version of the 2048 game.
 *
 * This function creates an instance of the `GuiGame` class and calls its `run` method
 * to initialize the window and start the main game loop.
 */
int main() {
    tfe::gui::GuiGame game;
    game.run();
    return 0;
}