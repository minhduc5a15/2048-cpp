#pragma once
#include "../core/board.h"

namespace tfe::renderer {

    class ConsoleRenderer {
    public:
        void render(const tfe::core::Board& board);
        void clear();
        void showGameOver();

    private:
        // Helper để lấy màu dựa trên giá trị ô
        const char* getColor(int value);
        // Reset màu về mặc định
        const char* resetColor();
    };

}  // namespace tfe::renderer