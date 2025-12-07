#include "console-renderer.h"
#include <iostream>
#include <iomanip>

namespace tfe::renderer {

// Mã màu ANSI
const char* ANSI_RESET = "\033[0m";
const char* ANSI_BOLD = "\033[1m";

void ConsoleRenderer::clear() {
    // Xóa màn hình và đưa con trỏ về góc trái trên
    std::cout << "\033[2J\033[H";
}

const char* ConsoleRenderer::getColor(int value) {
    switch (value) {
        case 2:    return "\033[48;5;255m\033[38;5;0m"; // White Back, Black Text
        case 4:    return "\033[48;5;229m\033[38;5;0m"; // Light Yellow
        case 8:    return "\033[48;5;208m\033[38;5;255m"; // Orange
        case 16:   return "\033[48;5;196m\033[38;5;255m"; // Red
        case 32:   return "\033[48;5;160m\033[38;5;255m"; // Dark Red
        case 64:   return "\033[48;5;199m\033[38;5;255m"; // Pink
        case 128:  return "\033[48;5;123m\033[38;5;0m";   // Cyan
        case 256:  return "\033[48;5;46m\033[38;5;0m";    // Green
        case 512:  return "\033[48;5;27m\033[38;5;255m";  // Blue
        case 1024: return "\033[48;5;226m\033[38;5;0m";   // Yellow
        case 2048: return "\033[48;5;220m\033[38;5;0m";   // Gold
        default:   return "\033[48;5;237m\033[38;5;255m"; // Dark Grey (Empty)
    }
}

const char* ConsoleRenderer::resetColor() {
    return ANSI_RESET;
}

void ConsoleRenderer::render(const tfe::core::Board& board) {
    clear();
    const auto& grid = board.getGrid();
    int size = board.getSize();

    std::cout << ANSI_BOLD << "   2048 - C++ Console Edition" << resetColor() << "\n\n";

    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            int val = grid[i][j];
            std::cout << getColor(val);
            if (val == 0) {
                std::cout << "      "; // 6 spaces
            } else {
                // Căn giữa số trong khoảng 6 ký tự
                std::string s = std::to_string(val);
                int padding = (6 - s.length()) / 2;
                std::cout << std::string(padding, ' ') << s << std::string(6 - padding - s.length(), ' ');
            }
            std::cout << resetColor() << " ";
        }
        std::cout << "\n\n";
    }
    std::cout << "Controls: WASD or Arrows to move. Q to Quit.\n";
}

void ConsoleRenderer::showGameOver() {
    std::cout << "\n" << ANSI_BOLD << "\033[31mGAME OVER!\033[0m" << "\n";
}

} // namespace tfe::renderer