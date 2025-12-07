#pragma once
#include "../core/board.h"

namespace tfe::gui {

    class RaylibRenderer {
    public:
        RaylibRenderer();
        ~RaylibRenderer(); // Destructor sẽ đóng cửa sổ

        bool shouldClose() const;
        void draw(const tfe::core::Board& board);

    private:
        // Tính toán kích thước ô dựa trên màn hình
        float cellSize_;
    };

} // namespace tfe::gui