#pragma once
#include "../core/board.h"

namespace tfe::renderer {

    // A static utility class for rendering the game state to the console.
    class ConsoleRenderer {
    public:
        // Renders the current state of the game board to the console.
        static void render(const tfe::core::Board& board);

        // Clears the console screen.
        static void clear();

        // Displays the "Game Over" message on the console.
        static void showGameOver();

    private:
        // Helper to get an ANSI color code based on the tile's value.
        static const char* getColor(int value);

        // Helper to get the ANSI code to reset the text color to default.
        static const char* resetColor();
    };

}  // namespace tfe::renderer