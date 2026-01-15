#pragma once
#include <memory>

#include "../ai/expectimax_agent.h"
#include "../core/board.h"
#include "../input/input-handler.h"
#include "../renderer/console-renderer.h"

namespace tfe::game {

    /**
     * @class Game
     * @brief Manages the main game loop and orchestrates the different components for the console version.
     *
     * This class ties together the game board, input handler, and console renderer
     * to create the playable game.
     */
    class Game {
    public:
        /**
         * @brief Constructs a new Game instance.
         */
        Game();

        /**
         * @brief Starts and runs the main game loop.
         */
        void run();

    private:
        tfe::core::Board board_;
        tfe::input::InputHandler inputHandler_;
        tfe::renderer::ConsoleRenderer renderer_;
        std::unique_ptr<tfe::ai::Agent> aiAgent_;
        bool isRunning_;
        bool isAiMode_ = false;
    };

}  // namespace tfe::game