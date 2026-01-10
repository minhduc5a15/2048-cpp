#pragma once
#include "core/board.h"
#include "core/game-observer.h"
#include "core/game-saver.h"
#include "raylib-renderer.h"

namespace tfe::gui {

    /**
     * @class GuiGame
     * @brief The main application class for the Graphical User Interface (Raylib) version.
     *
     * This class implements the classic "Game Loop" pattern:
     * 1. Handle Input & Update State (Update)
     * 2. Render State to Screen (Draw)
     *
     * It also acts as a bridge (Controller) between the core Logic (`Board`) 
     * and the View (`RaylibRenderer`).
     * It implements `IGameObserver` to listen for board events (like moves, merges) 
     * and trigger the corresponding visual animations in the renderer.
     */
    class GuiGame final : public tfe::core::IGameObserver {
    public:
        /**
         * @brief Constructs the GUI Game.
         * Initializes the window, loads resources, and sets up the board.
         */
        GuiGame();

        /**
         * @brief Starts the main application loop.
         * Blocking call that runs until the window is closed.
         */
        void run();

        // --- IGameObserver Implementation ---
        // These methods allow the core Board to notify the GUI about logic events
        // so the GUI can play appropriate animations.

        void onTileSpawn(int r, int c, int value) override;
        void onTileMerge(int r, int c, int newValue) override;
        void onTileMove(int fromR, int fromC, int toR, int toC, int value) override;
        void onGameOver() override;
        void onGameReset() override;

    private:
        /**
         * @brief Handles user input and game logic updates.
         * Called once per frame.
         */
        void update();

        /**
         * @brief Renders the current frame.
         * Called once per frame after update().
         */
        void draw() const;

        /**
         * @brief Helper to draw the exit confirmation dialog overlay.
         */
        static void drawExitDialog() ;

        tfe::core::Board board_;
        RaylibRenderer renderer_;
        bool isGameOver_;
        
        // Tracks the current move direction to correctly map visual coordinates
        // if transformations are used (though typically handled by the renderer).
        tfe::core::Direction currentMoveDirection_;  

        bool showExitPrompt_ = false;
        bool shouldExitApp_ = false;
    };
}  // namespace tfe::gui
