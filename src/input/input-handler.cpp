#include "input-handler.h"

#include <iostream>

// --- WINDOWS SECTION ---
#ifdef _WIN32
#include <conio.h>
#include <windows.h>

namespace tfe::input {

    InputHandler::InputHandler() { setRawMode(true); }
    InputHandler::~InputHandler() { setRawMode(false); }

    void InputHandler::setRawMode(bool enable) {
        // On Windows using _getch(), no complex raw mode is needed
        // Just hide the cursor for a cleaner interface
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_CURSOR_INFO cursorInfo;
        GetConsoleCursorInfo(hConsole, &cursorInfo);
        cursorInfo.bVisible = !enable;
        SetConsoleCursorInfo(hConsole, &cursorInfo);
    }

    InputHandler::InputCommand InputHandler::readInput(long timeout_ms) {
        
        // Windows Arrow Key Codes
        enum WinKey {
            ARROW_UP = 72,
            ARROW_DOWN = 80,
            ARROW_LEFT = 75,
            ARROW_RIGHT = 77
        };

        // If a timeout is specified, wait for input
        if (timeout_ms >= 0) {
            long elapsed = 0;
            const long step = 10;
            while (elapsed < timeout_ms) {
                if (_kbhit()) break;
                Sleep(step);
                elapsed += step;
            }
            if (!_kbhit()) return InputCommand::None;
        } else {
            // Block indefinitely
            while (!_kbhit()) {
                Sleep(10);
            }
        }

        // _getch() blocks until a key is pressed, but we know one is ready
        int c = _getch();

        switch (c) {
            case 'w':
            case 'W':
                return InputCommand::MoveUp;
            case 's':
            case 'S':
                return InputCommand::MoveDown;
            case 'a':
            case 'A':
                return InputCommand::MoveLeft;
            case 'd':
            case 'D':
                return InputCommand::MoveRight;
            case 'q':
            case 'Q':
                return InputCommand::Quit;
            case 'p':
            case 'P':
                return InputCommand::ToggleAutoPlay;

            // Arrow keys on Windows return two codes: 0 or 224, followed by the key code
            case 0:
            case 224: {
                int arrow = _getch();
                switch (arrow) {
                    case ARROW_UP:
                        return InputCommand::MoveUp;  // Up
                    case ARROW_DOWN:
                        return InputCommand::MoveDown;  // Down
                    case ARROW_LEFT:
                        return InputCommand::MoveLeft;  // Left
                    case ARROW_RIGHT:
                        return InputCommand::MoveRight;  // Right
                }
                return InputCommand::None;
            }
        }
        return InputCommand::None;
    }
}  // namespace tfe::input

// --- LINUX / MACOS SECTION ---
#else
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>

namespace tfe::input {

    static struct termios orig_termios;

    InputHandler::InputHandler() { setRawMode(true); }
    InputHandler::~InputHandler() { setRawMode(false); }

    void InputHandler::setRawMode(const bool enable) {
        if (enable) {
            tcgetattr(STDIN_FILENO, &orig_termios);
            struct termios raw = orig_termios;
            raw.c_lflag &= ~(ECHO | ICANON);
            tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
        } else {
            tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
        }
    }

    InputHandler::InputCommand InputHandler::readInput(long timeout_ms) {
        // Use select() to check for input without blocking.
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);

        struct timeval timeout;
        struct timeval* pTimeout = nullptr;

        if (timeout_ms >= 0) {
            timeout.tv_sec = timeout_ms / 1000;
            timeout.tv_usec = (timeout_ms % 1000) * 1000;
            pTimeout = &timeout;
        }

        int ready = select(STDIN_FILENO + 1, &fds, nullptr, nullptr, pTimeout);
        if (ready <= 0) {
            // Nothing to read, or an error occurred.
            return InputCommand::None;
        }

        char c;
        if (read(STDIN_FILENO, &c, 1) == -1) return InputCommand::None;

        switch (c) {
            case 'w':
            case 'W':
                return InputCommand::MoveUp;
            case 's':
            case 'S':
                return InputCommand::MoveDown;
            case 'a':
            case 'A':
                return InputCommand::MoveLeft;
            case 'd':
            case 'D':
                return InputCommand::MoveRight;
            case 'q':
            case 'Q':
                return InputCommand::Quit;
            case 'p':
            case 'P':
                return InputCommand::ToggleAutoPlay;
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
                            break;
                    }
                }
                return InputCommand::None;
            }
            default:
                return InputCommand::None;
        }
    }
}  // namespace tfe::input
#endif