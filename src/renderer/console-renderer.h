#pragma once
#include "../core/board.h"

namespace tfe::renderer {

    class ConsoleRenderer {
    public:
        static void render(const tfe::core::Board& board);
        static void clear();
        static void showGameOver();

    private:
        // Helper để lấy màu dựa trên giá trị ô
        static const char* getColor(int value);
        // Reset màu về mặc định
        static const char* resetColor();
    };

}  // namespace tfe::renderer