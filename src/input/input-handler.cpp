#include "input-handler.h"

#include <iostream>

// --- WINDOWS IMPLEMENTATION ---
#ifdef _WIN32
#include <conio.h>
#include <windows.h>

namespace tfe::input {

    InputHandler::InputHandler() { 
        // Enable raw mode (hide cursor) on initialization
        setRawMode(true); 
    }
    
    InputHandler::~InputHandler() { 
        // Restore normal mode on destruction
        setRawMode(false); 
    }

    void InputHandler::setRawMode(bool enable) {
        // On Windows using _getch(), true "raw mode" is implicit.
        // We mainly just want to hide the cursor for a cleaner CLI UI.
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_CURSOR_INFO cursorInfo;
        GetConsoleCursorInfo(hConsole, &cursorInfo);
        cursorInfo.bVisible = !enable; // Hide cursor when "raw mode" is enabled
        SetConsoleCursorInfo(hConsole, &cursorInfo);
    }

    InputHandler::InputCommand InputHandler::readInput() {
        // _getch() blocks execution until a key is pressed.
        // It does not echo the character to the console.
        int c = _getch();

        switch (c) {
            case 'w': case 'W': return InputCommand::MoveUp;
            case 's': case 'S': return InputCommand::MoveDown;
            case 'a': case 'A': return InputCommand::MoveLeft;
            case 'd': case 'D': return InputCommand::MoveRight;
            case 'q': case 'Q': return InputCommand::Quit;
            case 'p': case 'P': return InputCommand::AutoPlay;

            // Handle Arrow Keys
            // On Windows, arrow keys send two codes: 0 or 224, followed by the scan code.
            case 0:
            case 224: {
                int arrow = _getch(); // Read the second code
                switch (arrow) {
                    case 72: return InputCommand::MoveUp;
                    case 80: return InputCommand::MoveDown;
                    case 75: return InputCommand::MoveLeft;
                    case 77: return InputCommand::MoveRight;
                }
                return InputCommand::None;
            }
        }
        return InputCommand::None;
    }
}  // namespace tfe::input

// --- POSIX (LINUX / MACOS) IMPLEMENTATION ---
#else
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>

namespace tfe::input {

    // Store original terminal settings to restore them later
    static struct termios orig_termios;

    InputHandler::InputHandler() { setRawMode(true); }
    InputHandler::~InputHandler() { setRawMode(false); }

    void InputHandler::setRawMode(const bool enable) {
        if (enable) {
            // Get current terminal attributes
            tcgetattr(STDIN_FILENO, &orig_termios);
            struct termios raw = orig_termios;
            
            // Disable ECHO: Don't print typed characters
            // Disable ICANON: Disable canonical mode (line-by-line input).
            //                 Input is available immediately, not after Enter.
            raw.c_lflag &= ~(ECHO | ICANON);
            
            // Apply changes
            tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
        } else {
            // Restore original attributes
            tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
        }
    }

    InputHandler::InputCommand InputHandler::readInput() {
        // Use select() to check if input is available without blocking indefinitely.
        // This is useful if we wanted to implement a non-blocking loop later,
        // though currently we just wait.
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);

        // Wait indefinitely for input
        int ready = select(STDIN_FILENO + 1, &fds, nullptr, nullptr, nullptr);
        if (ready <= 0) {
            return InputCommand::None;
        }

        char c;
        // Read 1 byte
        if (read(STDIN_FILENO, &c, 1) == -1) return InputCommand::None;

        switch (c) {
            case 'w': case 'W': return InputCommand::MoveUp;
            case 's': case 'S': return InputCommand::MoveDown;
            case 'a': case 'A': return InputCommand::MoveLeft;
            case 'd': case 'D': return InputCommand::MoveRight;
            case 'q': case 'Q': return InputCommand::Quit;
            case 'p': case 'P': return InputCommand::AutoPlay;

            // Handle Escape Sequences (Arrow Keys)
            // Arrow keys usually send a sequence like: ESC [ A (Up), ESC [ B (Down)
            case '\033': { // Escape character
                char seq[2];
                // Try to read the next two bytes (e.g., [ and A)
                if (read(STDIN_FILENO, &seq[0], 1) == -1) return InputCommand::None;
                if (read(STDIN_FILENO, &seq[1], 1) == -1) return InputCommand::None;

                if (seq[0] == '[') {
                    switch (seq[1]) {
                        case 'A': return InputCommand::MoveUp;
                        case 'B': return InputCommand::MoveDown;
                        case 'C': return InputCommand::MoveRight;
                        case 'D': return InputCommand::MoveLeft;
                        default: break;
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