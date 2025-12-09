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

    InputHandler::InputCommand InputHandler::readInput() {

        // _getch() blocks until a key is pressed
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

            // Arrow keys on Windows return two codes: 0 or 224, followed by the key code
            case 0:
            case 224: {
                int arrow = _getch();
                switch (arrow) {
                    case 72:
                        return InputCommand::MoveUp;  // Up
                    case 80:
                        return InputCommand::MoveDown;  // Down
                    case 75:
                        return InputCommand::MoveLeft;  // Left
                    case 77:
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

    InputHandler::InputCommand InputHandler::readInput() {
        // Use select() to check for input without blocking.
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);

        // struct timeval timeout{};
        // timeout.tv_sec = 0;
        // timeout.tv_usec = 0;

        int ready = select(STDIN_FILENO + 1, &fds, nullptr, nullptr, nullptr);
        if (ready <= 0) {
            // Nothing to read, or an error occurred.
            return InputCommand::None;
        }

        char c;
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