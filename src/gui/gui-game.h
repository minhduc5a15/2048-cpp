#pragma once
#include "../core/board.h"
#include "raylib-renderer.h"

namespace tfe::gui {

    class GuiGame {
    public:
        GuiGame();
        void run();

    private:
        void update();
        void draw();
        void handleInput();

        tfe::core::Board board_;
        RaylibRenderer renderer_;
        bool isGameOver_;
    };

} // namespace tfe::gui