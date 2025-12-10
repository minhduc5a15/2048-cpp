#include "game.h"

#include <chrono>
#include <thread>

#include "core/ai_solver.h"
#include "score/score-manager.h"

namespace tfe::game {

    /**
     * @brief Constructor for the Game class.
     *
     * Initializes the game with a 4x4 board and sets the running state to true.
     */
    Game::Game() : board_(4), isRunning_(true) {}

    /**
     * @brief Runs the main game loop for the console version.
     *
     * This loop continues as long as the game is running. In each iteration, it:
     * 1. Renders the current state of the board to the console.
     * 2. Checks if the game is over. If so, it saves the score, displays the game over message, and waits for input before exiting.
     * 3. Reads user input for the next move or to quit.
     * 4. Updates the game state based on the user's command (moving tiles or quitting).
     * After the loop ends (e.g., user quits), it cleans up the console screen.
     */
    void Game::run() {
        bool needRender = true;

        while (isRunning_) {
            if (needRender) {
                tfe::renderer::ConsoleRenderer::render(board_);
                needRender = false;
            }

            // 1. Render the current board state.
            tfe::renderer::ConsoleRenderer::render(board_);

            // 2. Check for game over condition.
            if (board_.isGameOver()) {
                tfe::score::ScoreManager::save_game(board_.getScore(), board_.hasWon());
                tfe::renderer::ConsoleRenderer::showGameOver();
                // Wait for any key press to exit or handle restart logic.
                // For now, it just exits.
                tfe::input::InputHandler::readInput();
                break;
            }

            // 3. Read user input.
            const auto command = tfe::input::InputHandler::readInput();

            // 4. Update game logic based on input.
            bool moved = false;
            switch (command) {
                case input::InputHandler::InputCommand::Quit:
                    isRunning_ = false;
                    break;
                case input::InputHandler::InputCommand::MoveUp:
                    moved = board_.move(core::Direction::Up);
                    break;
                case input::InputHandler::InputCommand::MoveDown:
                    moved = board_.move(core::Direction::Down);
                    break;
                case input::InputHandler::InputCommand::MoveLeft:
                    moved = board_.move(core::Direction::Left);
                    break;
                case input::InputHandler::InputCommand::MoveRight:
                    moved = board_.move(core::Direction::Right);
                    break;

                case input::InputHandler::InputCommand::AutoPlay: {
                    while (!board_.isGameOver() && isRunning_) {
                        const auto bestDir = tfe::core::AISolver::findBestMove(board_, 12);

                        const bool aiMoved = board_.move(bestDir);

                        tfe::renderer::ConsoleRenderer::render(board_, true);

                        // fast, fast, fast and fast
                        std::this_thread::sleep_for(std::chrono::milliseconds(0));

                        if (!aiMoved) {
                            // AI got stuck
                            isRunning_ = false;
                            throw std::runtime_error("AI made an invalid move or got stuck. Exiting autoplay.");
                        }
                    }
                    needRender = true;
                    break;
                }
                default:
                    // If the user presses an invalid key or a move doesn't change the board,
                    // the loop will simply re-render and wait for the next input.
                    break;
            }

            if (moved) {
                needRender = true;
            }
        }

        tfe::renderer::ConsoleRenderer::clear();  // Clean up the screen on exit.
    }
}  // namespace tfe::game
