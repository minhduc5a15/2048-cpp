#include "gui-game.h"
#include "raylib.h"
#include "theme.h"

#include <map>

// Namespace for the GUI version of the 2048 game

namespace tfe::gui {

    // Structure to snapshot tile state for animation tracking
    struct TileSnapshot {
        int id;   // Tile ID
        int val;  // Tile value
        int r, c; // Row and column
    };

    /**
     * @brief Constructor for the GuiGame class.
     *
     * Initializes the game with a 4x4 board and sets the game over state to false.
     * It also sets up the Raylib renderer.
     */
    GuiGame::GuiGame() : board_(4), renderer_(), isGameOver_(false) {}

    /**
     * @brief Runs the main game loop for the GUI version.
     *
     * This loop continues as long as the window is open. It calls the update and draw methods
     * in each iteration to handle game logic and rendering.
     */
    void GuiGame::run() {
        while (!tfe::gui::RaylibRenderer::shouldClose()) {
            update();
            draw();
        }
    }

    /**
     * @brief Draws the entire game screen.
     *
     * This method is responsible for all rendering. It begins a drawing session,
     * tells the renderer to draw the board, and then, if the game is over,
     * it draws a semi-transparent overlay with the "GAME OVER" message.
     */
    void GuiGame::draw() const {
        BeginDrawing();
        renderer_.draw(board_);

        if (isGameOver_) {
            // Draw a semi-transparent overlay for the game over screen.
            DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(Theme::BG_COLOR, 0.8f));
            DrawText("GAME OVER", 80, 250, 60, Theme::TEXT_DARK);
            DrawText("Press ENTER to Restart", 120, 320, 20, Theme::TEXT_DARK);
        }
        EndDrawing();
    }

    /**
     * @brief Updates the game state based on user input and game logic.
     *
     * This method is called once per frame. It performs the following actions:
     * 1. Updates any ongoing animations. If an animation is playing, it returns early to prevent other actions.
     * 2. If the game is over, it listens for the ENTER key to restart the game.
     * 3. If the game is active, it listens for player input (Arrow keys or WASD) to determine the move direction.
     * 4. If a move is made, it captures the board state before the move to detect changes for animation.
     * 5. It then executes the move on the board.
     * 6. If the move caused changes, it triggers animations for sliding or merged tiles.
     * 7. It checks for a game over condition after the move.
     * 8. It checks if a new tile has spawned and triggers the spawn animation.
     */
    void GuiGame::update() {
        // First, update any ongoing animations. Don't process input while animating.
        renderer_.updateAnimation(GetFrameTime());
        if (renderer_.isAnimating()) return;

        // If the game is over, check for the restart command.
        if (isGameOver_) {
            if (IsKeyPressed(KEY_ENTER)) {
                board_.reset();
                isGameOver_ = false;
            }
            return;
        }

        // Detect user input for movement.
        tfe::core::Direction dir = {};
        bool pressed = false;
        if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
            dir = tfe::core::Direction::Up;
            pressed = true;
        } else if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
            dir = tfe::core::Direction::Down;
            pressed = true;
        } else if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) {
            dir = tfe::core::Direction::Left;
            pressed = true;
        } else if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) {
            dir = tfe::core::Direction::Right;
            pressed = true;
        }

        if (pressed) {
            // Before moving, create a snapshot of the current tile positions and IDs.
            // This is used later to determine which tiles have moved or merged for animation.
            std::map<int, TileSnapshot> oldStates;
            const int size = board_.getSize();
            for (int r = 0; r < size; ++r) {
                for (int c = 0; c < size; ++c) {
                    if (int id = board_.getTileId(r, c); id != 0) oldStates[id] = {id, board_.getTile(r, c), r, c};
                }
            }

            // Execute the move on the game board.
            const bool moved = board_.move(dir);

            // If the board state changed, figure out and trigger animations.
            if (moved) {
                auto spawnPos = board_.getLastSpawnPos();

                // Iterate through the new board state to find what changed.
                for (int r = 0; r < size; ++r) {
                    for (int c = 0; c < size; ++c) {
                        int id = board_.getTileId(r, c);
                        if (id == 0) continue;

                        if (oldStates.contains(id)) {
                            // This tile existed before. Check if it slid to a new position.
                            TileSnapshot old = oldStates[id];
                            if (old.r != r || old.c != c) {
                                renderer_.addMovingTile(old.val, id, old.r, old.c, r, c);
                            }
                        } else {
                            // This is a new tile ID, which means a merge happened at this spot.
                            // We trigger a merge animation, unless this is the location of the newly spawned tile.
                            if (r != spawnPos.r || c != spawnPos.c) {
                                renderer_.triggerMerge(r, c);
                            }
                        }
                    }
                }
            }

            // After a move, check if the game is over.
            if (board_.isGameOver()) isGameOver_ = true;
        }

        // After any potential move, check if a new tile has spawned.
        // This is done by comparing the last seen spawn position with the current one.
        if (const auto currentSpawnPos = board_.getLastSpawnPos();
            currentSpawnPos.r != lastSeenSpawnPos_.r || currentSpawnPos.c != lastSeenSpawnPos_.c) {
            if (currentSpawnPos.r != -1) {
                // A new tile has appeared, trigger the spawn animation.
                renderer_.triggerSpawn(currentSpawnPos.r, currentSpawnPos.c);
            }
            lastSeenSpawnPos_ = currentSpawnPos;
        }
    }
}  // namespace tfe::gui