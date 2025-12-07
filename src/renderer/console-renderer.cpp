#include "console-renderer.h"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

// Namespace for the Text-based Fantasy Engine (TFE) renderer components, specifically for console-based rendering

namespace tfe::renderer {

    // ANSI escape sequences for resetting styles and enabling bold text.
    auto ANSI_RESET = "\033[0m";
    auto ANSI_BOLD = "\033[1m";

    /**
     * @brief Sets up the console for proper UTF-8 output and ANSI escape sequence support.
     *
     * This is particularly important on Windows, where it enables virtual terminal processing,
     * allowing the console to interpret ANSI color and formatting codes correctly.
     */
    void setupConsole() {
#ifdef _WIN32
        // Set the console output code page to UTF-8 to ensure proper character encoding.
        SetConsoleOutputCP(CP_UTF8);

        // Enable ANSI escape sequences by modifying the console mode.
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD dwMode = 0;
        GetConsoleMode(hOut, &dwMode);
        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING; // Enable virtual terminal processing.
        SetConsoleMode(hOut, dwMode);
#endif
    }

    /**
     * @brief A helper struct to ensure the console is set up automatically at program start.
     *
     * The static instance of this struct will call setupConsole() once during static initialization.
     */
    struct ConsoleInitializer {
        ConsoleInitializer() { setupConsole(); }
    };

    // Static instance that triggers the console setup.
    static ConsoleInitializer consoleInit;

    /**
     * @brief Clears the console screen and moves the cursor to the top-left corner.
     *
     * It uses ANSI escape sequences for cross-platform compatibility.
     * `\033[2J` clears the entire screen.
     * `\033[H` moves the cursor to the home position (top-left).
     */
    void ConsoleRenderer::clear() {
        std::cout << "\033[2J\033[H";
    }

    /**
     * @brief Gets the ANSI color code for a tile based on its value.
     * @param value The value of the tile (e.g., 2, 4, 8).
     * @return A C-string containing the ANSI escape code for the appropriate background and foreground colors.
     */
    const char* ConsoleRenderer::getColor(const int value) {
        switch (value) {
            case 2:
                return "\033[48;5;255m\033[38;5;0m";  // White background, black text
            case 4:
                return "\033[48;5;229m\033[38;5;0m";  // Light yellow background, black text
            case 8:
                return "\033[48;5;208m\033[38;5;255m";  // Orange background, white text
            case 16:
                return "\033[48;5;196m\033[38;5;255m";  // Red background, white text
            case 32:
                return "\033[48;5;160m\033[38;5;255m";  // Dark red background, white text
            case 64:
                return "\033[48;5;199m\033[38;5;255m";  // Pink background, white text
            case 128:
                return "\033[48;5;123m\033[38;5;0m";  // Cyan background, black text
            case 256:
                return "\033[48;5;46m\033[38;5;0m";  // Green background, black text
            case 512:
                return "\033[48;5;27m\033[38;5;255m";  // Blue background, white text
            case 1024:
                return "\033[48;5;226m\033[38;5;0m";  // Yellow background, black text
            case 2048:
                return "\033[48;5;220m\033[38;5;0m";  // Gold background, black text
            default:
                return "\033[48;5;237m\033[38;5;255m";  // Dark grey background (for empty tiles), white text
        }
    }

    /**
     * @brief Gets the ANSI reset code to revert to default console colors and styles.
     * @return A C-string containing the ANSI reset escape code.
     */
    const char* ConsoleRenderer::resetColor() { return ANSI_RESET; }

    /**
     * @brief Renders the entire game board to the console.
     *
     * This function clears the screen, prints the title, then iterates through the grid
     * to draw each tile with its corresponding color and value. Finally, it displays the controls.
     * @param board The game board to be rendered.
     */
    void ConsoleRenderer::render(const tfe::core::Board& board) {
        // Clear the console before rendering the new frame.
        clear();
        // Retrieve the grid data and its size from the board.
        const auto& grid = board.getGrid();
        const int size = board.getSize();

        // Build and print the header with score information
        std::stringstream header;
        header << ANSI_BOLD << "2048" << resetColor() << "   |   SCORE: " << board.getScore()
               << "   |   BEST: " << board.getHighScore();
        std::cout << header.str() << "\n\n";

        // Loop through each row and column to render the grid.
        for (int i = 0; i < size; ++i) {
            for (int j = 0; j < size; ++j) {
                const int val = grid[i][j];
                // Set the color for the current tile.
                std::cout << getColor(val);
                if (val == 0) {
                    // For empty tiles, print spaces to maintain cell width.
                    std::cout << "      ";
                } else {
                    // Center the number within a fixed-width cell.
                    std::string s = std::to_string(val);
                    const int padding = static_cast<int>(6 - s.length()) / 2;
                    std::cout << std::string(padding, ' ') << s << std::string(6 - padding - s.length(), ' ');
                }
                // Reset color after each cell and add a space for separation.
                std::cout << resetColor() << " ";
            }
            // Add extra newlines for vertical spacing between rows.
            std::cout << "\n\n";
        }
        // Print control instructions at the bottom.
        std::cout << "Controls: WASD or Arrows to move. Q to Quit.\n";
    }

    /**
     * @brief Displays a "Game Over" message in bold red text.
     */
    void ConsoleRenderer::showGameOver() { std::cout << "\n" << ANSI_BOLD << "\033[31mGAME OVER!\033[0m" << "\n"; }

}  // namespace tfe::renderer