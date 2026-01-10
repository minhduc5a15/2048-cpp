#include "game.h"

#include <chrono>
#include <thread>

#include "core/ai_solver.h"
#include "score/score-manager.h"

namespace tfe::game {

    /**
     * @brief Constructor for the Game class.
     *
     * Initializes the game with a standard 4x4 board and sets the initial state to running.
     * The board initialization includes seeding the random generator and spawning start tiles.
     */
    Game::Game() : board_(4), isRunning_(true) {}

    /**
     * @brief Executes the main game loop for the console-based 2048 game.
     *
     * This method manages the lifecycle of the game application:
     * 1. Rendering: Displays the board using ASCII/ANSI art.
     * 2. Input: Waits for and processes user keystrokes.
     * 3. Logic: Updates the board state based on moves or AI commands.
     * 4. Game Over: Handles win/loss conditions and score saving.
     */
    void Game::run() {
        bool needRender = true;

        while (isRunning_) {
            // Optimize rendering: only redraw when the state changes.
            if (needRender) {
                tfe::renderer::ConsoleRenderer::render(board_);
                needRender = false;
            }

            // Force render before checking game over to ensure final state is visible.
            // (Note: The double render here in the original logic is slightly redundant but harmless)
            tfe::renderer::ConsoleRenderer::render(board_);

            // Check for Game Over condition
            if (board_.isGameOver()) {
                // Save score and display end screen
                tfe::score::ScoreManager::save_game(board_.getScore(), board_.hasWon());
                tfe::renderer::ConsoleRenderer::showGameOver();
                
                // Wait for user acknowledgment before exiting
                tfe::input::InputHandler::readInput();
                break;
            }

            // Block and wait for user input
            const auto command = tfe::input::InputHandler::readInput();

            // Process the command
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
                    // Activate AI Auto-player
                    while (!board_.isGameOver() && isRunning_) {
                        // AI thinks with a depth limit (12 is a reasonable balance for speed/accuracy)
                        const auto bestDir = tfe::core::AISolver::findBestMove(board_, 12);

                        const bool aiMoved = board_.move(bestDir);

                        // Visualize the AI's move
                        tfe::renderer::ConsoleRenderer::render(board_, true);

                        // Optional delay to make AI moves followable by human eyes
                        std::this_thread::sleep_for(std::chrono::milliseconds(0));

                        if (!aiMoved) {
                            // Safety check: AI shouldn't return a move that doesn't change the board
                            isRunning_ = false;
                            throw std::runtime_error("AI made an invalid move or got stuck. Exiting autoplay.");
                        }
                    }
                    needRender = true;
                    break;
                }
                default:
                    // Invalid input: ignore and wait for next input
                    break;
            }

            if (moved) {
                needRender = true;
            }
        }

        // Cleanup resources
        tfe::renderer::ConsoleRenderer::clear(); 
    }
}  // namespace tfe::game
