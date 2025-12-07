#pragma once
#include "../core/board.h"
#include "raylib-renderer.h"

namespace tfe::gui {

    /**
     * @class GuiGame
     * @brief Manages the main game loop and state for the GUI version of 2048.
     *
     * This class orchestrates the interaction between the core game logic (`Board`)
     * and the GUI renderer (`RaylibRenderer`), handling user input, game state updates,
     * and rendering each frame.
     */
    class GuiGame {
    public:
        /**
         * @brief Constructs a new GuiGame instance.
         */
        GuiGame();

        /**
         * @brief Starts and runs the main game loop.
         */
        void run();

    private:
        /**
         * @brief Updates the game state for the current frame.
         */
        void update();

        /**
         * @brief Draws the current game state to the screen.
         */
        void draw() const;

        tfe::core::Board board_;       // The core game board logic and state.
        RaylibRenderer renderer_;      // The renderer for drawing the GUI.
        bool isGameOver_;              // Flag indicating if the game over condition has been met.
        // Tracks the last seen spawn position to trigger spawn animations correctly.
        tfe::core::Board::Position lastSeenSpawnPos_ = {-1, -1};
    };
}  // namespace tfe::gui
