#include "game.h"

namespace tfe::game {
    Game::Game() : board_(4), isRunning_(true) {}

    void Game::run() {
        while (isRunning_) {
            // 1. Render
            renderer_.render(board_);

            // 2. Check Game Over
            if (board_.isGameOver()) {
                renderer_.showGameOver();
                // Chờ người dùng nhấn phím bất kỳ để thoát hoặc xử lý logic
                // restart (tạm thời thoát)
                inputHandler_.readInput();
                break;
            }

            // 3. Input
            const auto command = inputHandler_.readInput();

            // 4. Update Logic
            switch (command) {
                case input::InputHandler::InputCommand::Quit:
                    isRunning_ = false;
                    break;
                case input::InputHandler::InputCommand::MoveUp:
                    board_.move(core::Direction::Up);
                    break;
                case input::InputHandler::InputCommand::MoveDown:
                    board_.move(core::Direction::Down);
                    break;
                case input::InputHandler::InputCommand::MoveLeft:
                    board_.move(core::Direction::Left);
                    break;
                case input::InputHandler::InputCommand::MoveRight:
                    board_.move(core::Direction::Right);
                    break;
                default:
                    break;
            }

            // Nếu người dùng bấm phím không hợp lệ hoặc di chuyển không tạo ra
            // thay đổi, vòng lặp sẽ render lại và chờ tiếp.
        }

        renderer_.clear();  // Dọn dẹp màn hình khi thoát
    }
}  // namespace tfe::game
