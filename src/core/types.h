#pragma once
#include <cstdint>
#include <vector>

namespace tfe::core {
    // Tile lưu số mũ (0..15). Ví dụ: 1 -> 2, 2 -> 4. 0 là ô trống.
    using Tile = uint8_t;

    // Bitboard 64-bit chứa toàn bộ bàn cờ 4x4.
    // Cấu trúc bộ nhớ: [Hàng 3][Hàng 2][Hàng 1][Hàng 0] (Mỗi hàng 16 bit)
    using Bitboard = uint64_t;

    // Một hàng 4 ô (4x4 bit = 16 bit)
    using Row = uint16_t;

    // Grid cũ vẫn giữ để dùng cho GUI (render giá trị thực ra màn hình)
    using Grid = std::vector<std::vector<int>>;

    enum class Direction { Up, Down, Left, Right };

    struct GameState {
        Bitboard board;
        int score;
    };
}  // namespace tfe::core
