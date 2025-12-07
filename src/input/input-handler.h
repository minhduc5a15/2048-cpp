#pragma once
#include "../core/types.h"

namespace tfe::input {

    class InputHandler {
    public:
        InputHandler();
        ~InputHandler();

        // Trả về hướng di chuyển hoặc null nếu không phải phím điều hướng
        // Trả về phương thức input để Game Loop xử lý
        enum class InputCommand {
            None,
            MoveUp,
            MoveDown,
            MoveLeft,
            MoveRight,
            Quit
        };

        InputCommand readInput();

    private:
        void setRawMode(bool enable);
    };

}  // namespace tfe::input