#pragma once
#include "../core/board.h"
#include "../input/input-handler.h"
#include "../renderer/console-renderer.h"

namespace tfe::game {

    class Game {
    public:
        Game();
        void run();

    private:
        tfe::core::Board board_;
        tfe::input::InputHandler inputHandler_;
        tfe::renderer::ConsoleRenderer renderer_;
        bool isRunning_;
    };

}  // namespace tfe::game