#pragma once

namespace tfe::input {

    /**
     * @class InputHandler
     * @brief Handles raw keyboard input for the console application.
     *
     * This class is responsible for setting the terminal to raw mode to capture
     * key presses without waiting for an Enter key, and then interpreting
     * those key presses as game commands.
     */
    class InputHandler {
    public:
        InputHandler();
        ~InputHandler();

        // Enum representing the high-level commands that can be issued by the user.
        enum class InputCommand { None, MoveUp, MoveDown, MoveLeft, MoveRight, Quit, ToggleAutoPlay };

        /**
         * @brief Reads and interprets the next keyboard input from the user.
         * @param timeout_ms The maximum time to wait for input in milliseconds. -1 means wait indefinitely.
         * @return The corresponding InputCommand for the key that was pressed, or None if timed out.
         */
        static InputCommand readInput(long timeout_ms = -1);

    private:
        /**
         * @brief Enables or disables raw mode for the terminal.
         *
         * In raw mode, characters are read directly without buffering, allowing for
         * immediate capture of keys like arrows.
         * @param enable True to enable raw mode, false to disable it.
         */
        static void setRawMode(bool enable);
    };

}  // namespace tfe::input