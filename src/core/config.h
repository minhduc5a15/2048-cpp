#pragma once
#include <cstdint>

namespace tfe::core::Config {
    // Defines the rules and parameters of the game.

    constexpr int DEFAULT_BOARD_SIZE = 4;

    // 2^11 = 2048. AI cần đạt được số mũ 11.
    constexpr int WINNING_EXPONENT = 11;

    constexpr double SPAWN_PROBABILITY_2 = 0.9;

    // Giá trị số mũ khi sinh ra
    constexpr int TILE_EXPONENT_LOW = 1;   // 2^1 = 2
    constexpr int TILE_EXPONENT_HIGH = 2;  // 2^2 = 4

    // Bitmask lấy 16 bit cuối (đại diện cho một hàng)
    constexpr uint64_t ROW_MASK = 0xFFFFULL;

    // Bitmask lấy cột (dùng cho debug hoặc các phép toán cột phức tạp)
    constexpr uint64_t COL_MASK = 0x000F000F000F000FULL;
}  // namespace tfe::core::Config