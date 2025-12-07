#include "input-handler.h"

#include <termios.h>
#include <unistd.h>

namespace tfe::input {

    // Stores the original terminal settings to restore them when the game exits.
    static struct termios orig_termios;

    // Constructor: enables raw mode when an InputHandler is created.
    InputHandler::InputHandler() { setRawMode(true); }

    // Destructor: ensures raw mode is disabled when the InputHandler is destroyed.
    InputHandler::~InputHandler() { setRawMode(false); }

    void InputHandler::setRawMode(const bool enable) {
        if (enable) {
            // Get the current terminal attributes.
            tcgetattr(STDIN_FILENO, &orig_termios);
            struct termios raw = orig_termios;

            // Disable echo (so typed characters don't appear) and
            // canonical mode (to read input immediately instead of waiting for Enter).
            raw.c_lflag &= ~(ECHO | ICANON);

            // Apply the new (raw) terminal settings.
            tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
        } else {
            // Restore the original terminal settings.
            tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
        }
    }

    InputHandler::InputCommand InputHandler::readInput() {
        char c;
        // Read 1 byte from standard input.
        if (read(STDIN_FILENO, &c, 1) == -1) return InputCommand::None;

        switch (c) {
            // WASD controls
            case 'w':
                return InputCommand::MoveUp;
            case 's':
                return InputCommand::MoveDown;
            case 'a':
                return InputCommand::MoveLeft;
            case 'd':
                return InputCommand::MoveRight;
            // Quit command
            case 'q':
                return InputCommand::Quit;

            // Handle arrow keys, which send multi-byte ANSI escape sequences.
            // The sequence for an arrow key is typically '\033' (Escape), followed by '[' and then A, B, C, or D.
            case '\033': {
                char seq[2];
                // Read the next two bytes of the sequence.
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
                // Any other key is ignored.
                return InputCommand::None;
        }
    }

}  // namespace tfe::input