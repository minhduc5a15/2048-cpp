#include "game.h"

#include "score/score-manager.h"
#include <chrono>
#include <iostream>

namespace tfe::game {

    /**
     * @brief Constructor for the Game class.
     *
     * Initializes the game with a 4x4 board and sets the running state to true.
     */
    Game::Game() : board_(), isRunning_(true) {
        aiAgent_ = std::make_unique<tfe::ai::ExpectimaxAgent>();
    }

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

            // Check for game over condition.
            if (board_.isGameOver()) {
                tfe::score::ScoreManager::save_game(board_.getScore(), board_.hasWon());
                tfe::renderer::ConsoleRenderer::showGameOver();
                // Wait for any key press to exit or handle restart logic.
                tfe::input::InputHandler::readInput();
                break;
            }

            // Read user input. If AI mode is on, use a timeout (100ms) to allow for interruption.
            // If AI mode is off, block indefinitely.
            tfe::input::InputHandler::InputCommand command;
            if (isAiMode_) {
                command = tfe::input::InputHandler::readInput(100);
            } else {
                command = tfe::input::InputHandler::readInput(-1);
            }

            // Update game logic based on input.
            bool moved = false;

            // Handle Global Commands first
            if (command == input::InputHandler::InputCommand::Quit) {
                isRunning_ = false;
                break;
            }
            if (command == input::InputHandler::InputCommand::ToggleAutoPlay) {
                isAiMode_ = !isAiMode_;
                continue; // Skip the rest of the loop to process the toggle immediately
            }

            // Handle Move Commands
            switch (command) {
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
                case input::InputHandler::InputCommand::None:
                    // If no user input and AI mode is ON, let AI play
                    if (isAiMode_) {
                        auto bestMove = aiAgent_->getBestMove(board_.getState().board);
                        if (bestMove) {
                            moved = board_.move(*bestMove);
                        }
                    }
                    break;
                default:
                    break;
            }

            if (moved) {
                needRender = true;
            }
        }

        tfe::renderer::ConsoleRenderer::clear();  // Clean up the screen on exit.
    }
}  // namespace tfe::game
