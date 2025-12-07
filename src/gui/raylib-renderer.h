#pragma once
#include "../core/board.h"
#include <vector>

namespace tfe::gui {

    class RaylibRenderer {
    public:
        RaylibRenderer();
        ~RaylibRenderer();
        bool shouldClose() const;
        void draw(const tfe::core::Board& board);

        void updateAnimation(float dt);

        // Kích hoạt hiệu ứng cho ô tại vị trí r, c
        void triggerSpawnAnimation(int r, int c);

    private:
        float cellSize_;

        std::vector<std::vector<float>> cellScales_;
    };

} // namespace tfe::gui