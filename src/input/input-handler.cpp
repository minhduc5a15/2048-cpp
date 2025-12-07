#include "input-handler.h"

#include <termios.h>
#include <unistd.h>

namespace tfe::input {

    // Lưu trạng thái terminal gốc để khôi phục khi thoát game
    static struct termios orig_termios;

    InputHandler::InputHandler() { setRawMode(true); }

    InputHandler::~InputHandler() { setRawMode(false); }

    void InputHandler::setRawMode(const bool enable) {
        if (enable) {
            tcgetattr(STDIN_FILENO, &orig_termios);
            struct termios raw = orig_termios;
            // Tắt echo (không hiện ký tự khi gõ) và canonical mode (đọc ngay
            // lập tức)
            raw.c_lflag &= ~(ECHO | ICANON);
            tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
        } else {
            tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
        }
    }

    InputHandler::InputCommand InputHandler::readInput() {
        char c;
        // Đọc 1 byte từ stdin
        if (read(STDIN_FILENO, &c, 1) == -1) return InputCommand::None;

        switch (c) {
            case 'w':
                return InputCommand::MoveUp;
            case 's':
                return InputCommand::MoveDown;
            case 'a':
                return InputCommand::MoveLeft;
            case 'd':
                return InputCommand::MoveRight;
            case 'q':
                return InputCommand::Quit;
                // Xử lý phím mũi tên (ANSI escape sequence: ^[[A, ^[[B...)
            case '\033': {
                char seq[2];
                if (read(STDIN_FILENO, &seq[0], 1) == -1) return InputCommand::None;
                if (read(STDIN_FILENO, &seq[1], 1) == -1) return InputCommand::None;
                if (seq[0] == '[') {
                    switch (seq[1]) {
                        case 'A':
                            return InputCommand::MoveUp;
                        case 'B':
                            return InputCommand::MoveDown;
                        case 'C':
                            return InputCommand::MoveRight;
                        case 'D':
                            return InputCommand::MoveLeft;
                        default:
                            return InputCommand::None;
                    }
                }
                return InputCommand::None;
            }
            default:
                return InputCommand::None;
        }
    }

}  // namespace tfe::input