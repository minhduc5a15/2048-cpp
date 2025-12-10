#pragma once
#include "core/board.h"
#include "core/game-observer.h"
#include "core/game-saver.h"
#include "raylib-renderer.h"

namespace tfe::gui {

    /**
     * @class GuiGame
     * @brief Manages the main game loop and state for the GUI version of 2048.
     *
     * This class orchestrates the interaction between the core game logic (`Board`)
     * and the GUI renderer (`RaylibRenderer`), handling user input, game state updates,
     * and rendering each frame. It implements IGameObserver to react to board events.
     */
    class GuiGame final : public tfe::IGameObserver {
    public:
        GuiGame();
        void run();

        // --- IGameObserver Implementation ---
        void onTileSpawn(int r, int c, int value) override;
        void onTileMerge(int r, int c, int newValue) override;
        void onTileMove(int fromR, int fromC, int toR, int toC, int value) override;
        void onGameOver() override;
        void onGameReset() override;

    private:
        void update();
        void draw() const;

        static void drawExitDialog() ;

        tfe::core::Board board_;
        RaylibRenderer renderer_;
        bool isGameOver_;
        tfe::core::Direction currentMoveDirection_;  // To handle transformed coordinates

        bool showExitPrompt_ = false;
        bool shouldExitApp_ = false;
    };
}  // namespace tfe::gui
