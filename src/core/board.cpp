#include "board.h"
#include "config.h"
#include "score/score-manager.h"
#include "utils/random-generator.h"

#include <algorithm>
#include <stdexcept>

// Namespace for core components of the 2048 game
namespace tfe::core {

    /**
     * @brief Constructor for the Board class.
     *
     * Initializes a square board of a given size.
     * It validates the size to ensure it's at least 2x2.
     * After validation, it resets the board to its initial state.
     *
     * @param size The size of one dimension of the square board.
     * @throws std::invalid_argument if the size is less than 2.
     */
    Board::Board(const int size) : size_(size), score_(0), hasReachedWinTile_(false) {
        highScore_ = tfe::score::ScoreManager::load_high_score();
        if (size < 2) throw std::invalid_argument("Board size must be at least 2x2");
        reset();
    }

    /**
     * @brief Resets the board to the initial game state.
     *
     * This function clears the entire grid, setting all tiles to 0.
     * It also resets the tile ID grid and the ID counter.
     * Finally, it spawns two new random tiles to start the game.
     */
    void Board::reset() {
        grid_.assign(size_, std::vector<Tile>(size_, 0));
        idGrid_.assign(size_, std::vector<int>(size_, 0));
        nextId_ = 1;
        score_ = 0;
        hasReachedWinTile_ = false;
        spawnRandomTile();
        spawnRandomTile();
    }

    /**
     * @brief Gets the size of the board.
     *
     * @return The size (width or height) of the board.
     */
    int Board::getSize() const { return size_; }

    /**
     * @brief Gets the current state of the game grid.
     *
     * @return A constant reference to the 2D vector representing the grid.
     */
    const Grid& Board::getGrid() const { return grid_; }

    /**
     * @brief Gets the unique ID of a tile at a specific position.
     *
     * @param row The row index of the tile.
     * @param col The column index of the tile.
     * @return The ID of the tile, or 0 if the position is invalid or the tile is empty.
     */
    int Board::getTileId(const int row, const int col) const {
        if (row >= 0 && row < size_ && col >= 0 && col < size_) {
            return idGrid_[row][col];
        }
        return 0;
    }

    /**
     * @brief Sets the value of a tile at a specific position.
     *
     * If the value is not 0, a new unique ID is assigned to the tile.
     * If the value is 0, the tile's ID is also set to 0.
     *
     * @param row The row index.
     * @param col The column index.
     * @param value The new value for the tile.
     */
    void Board::setTile(const int row, const int col, const Tile value) {
        if (row >= 0 && row < size_ && col >= 0 && col < size_) {
            grid_[row][col] = value;
            if (value != 0) {
                idGrid_[row][col] = nextId_++;
            } else {
                idGrid_[row][col] = 0;
            }
        }
    }

    /**
     * @brief Gets the value of a tile at a specific position.
     *
     * @param row The row index.
     * @param col The column index.
     * @return The value of the tile, or -1 if the position is out of bounds.
     */
    Tile Board::getTile(const int row, const int col) const {
        if (row >= 0 && row < size_ && col >= 0 && col < size_) return grid_[row][col];
        return -1;
    }

    // --- HELPER FUNCTIONS ---

    /**
     * @brief Compresses a row by moving all non-zero tiles to the left.
     *
     * This helper function is used as part of the move logic. It takes a row
     * and its corresponding ID row, and shifts all non-zero tiles to the beginning,
     * filling the rest of the row with zeros.
     *
     * @param row The tile row to compress.
     * @param idRow The corresponding ID row to compress.
     */
    void Board::compress(std::vector<Tile>& row, std::vector<int>& idRow) const {
        std::vector<Tile> newRow(size_, 0);
        std::vector<int> newIdRow(size_, 0);
        int index = 0;
        for (int i = 0; i < size_; ++i) {
            if (row[i] != 0) {
                newRow[index] = row[i];
                newIdRow[index] = idRow[i];
                index++;
            }
        }
        row = newRow;
        idRow = newIdRow;
    }

    /**
     * @brief Merges adjacent identical tiles in a row.
     *
     * This function iterates through a row and if two adjacent tiles have the same value,
     * it merges them by doubling the value of the first tile and setting the second to 0.
     * A new ID is assigned to the merged tile.
     *
     * @param row The tile row to merge.
     * @param idRow The corresponding ID row to update.
     */
    void Board::merge(std::vector<Tile>& row, std::vector<int>& idRow) {
        for (int i = 0; i < size_ - 1; ++i) {
            if (row[i] != 0 && row[i] == row[i + 1]) {
                row[i] *= 2;
                row[i + 1] = 0;

                // Update score and high score
                score_ += row[i];
                if (score_ > highScore_) {
                    highScore_ = score_;
                }

                // Check for win condition
                if (row[i] == Config::WINNING_TILE) {
                    hasReachedWinTile_ = true;
                }

                idRow[i] = nextId_++;
                idRow[i + 1] = 0;
            }
        }
    }

    /**
     * @brief Executes the "move left" logic for the entire board.
     *
     * For each row, it compresses, merges, and then compresses again to handle
     * the gaps created by merging. It tracks whether any change occurred.
     *
     * @return True if the board state changed, false otherwise.
     */
    bool Board::moveLeft() {
        bool changed = false;
        for (int i = 0; i < size_; ++i) {
            std::vector<Tile> original = grid_[i];
            std::vector<int> originalIds = idGrid_[i];

            compress(grid_[i], idGrid_[i]);
            merge(grid_[i], idGrid_[i]);
            compress(grid_[i], idGrid_[i]);

            if (grid_[i] != original || idGrid_[i] != originalIds) {
                changed = true;
            }
        }
        return changed;
    }

    /**
     * @brief Reverses each row of the board.
     *
     * This is a helper transformation used to implement "move right" and "move down".
     */
    void Board::reverse() {
        for (int i = 0; i < size_; ++i) {
            std::ranges::reverse(grid_[i]);
            std::ranges::reverse(idGrid_[i]);
        }
    }

    /**
     * @brief Transposes the board.
     *
     * Swaps rows and columns. This is a helper transformation used to
     * implement "move up" and "move down".
     */
    void Board::transpose() {
        Grid newGrid(size_, std::vector<Tile>(size_, 0));
        std::vector<std::vector<int>> newIdGrid(size_, std::vector<int>(size_, 0));
        for (int i = 0; i < size_; ++i) {
            for (int j = 0; j < size_; ++j) {
                newGrid[i][j] = grid_[j][i];
                newIdGrid[i][j] = idGrid_[j][i];
            }
        }
        grid_ = newGrid;
        idGrid_ = newIdGrid;
    }

    /**
     * @brief Performs a move in a given direction.
     *
     * It uses a combination of transpose and reverse transformations to reuse
     * the `moveLeft` logic for all four directions.
     * If the move results in a change, a new random tile is spawned.
     *
     * @param dir The direction of the move (Up, Down, Left, Right).
     * @return True if the board state changed, false otherwise.
     */
    bool Board::move(const Direction dir) {
        clearEffects();
        bool changed = false;
        // The move logic is based on transforming the board so that every move
        // can be treated as a 'move left', then transforming it back.
        switch (dir) {
            case Direction::Left:
                changed = moveLeft();
                break;
            case Direction::Right:
                reverse();
                changed = moveLeft();
                reverse();
                break;
            case Direction::Up:
                transpose();
                changed = moveLeft();
                transpose();
                break;
            case Direction::Down:
                transpose();
                reverse();
                changed = moveLeft();
                reverse();
                transpose();
                break;
        }
        if (changed) spawnRandomTile();
        return changed;
    }

    /**
     * @brief Spawns a new random tile in an empty cell.
     *
     * It first finds all empty cells on the board. If any exist, it randomly
     * selects one and places a new tile (either 2 or 4, based on configuration)
     * in that cell. The position of the newly spawned tile is recorded.
     */
    void Board::spawnRandomTile() {
        std::vector<std::pair<int, int>> emptyCells;
        for (int i = 0; i < size_; ++i) {
            for (int j = 0; j < size_; ++j) {
                if (grid_[i][j] == 0) emptyCells.emplace_back(i, j);
            }
        }

        if (!emptyCells.empty()) {
            const int index = utils::RandomGenerator::getInt(0, static_cast<int>(emptyCells.size()) - 1);
            // Determine the value of the new tile (2 or 4) based on a configured probability.
            const Tile val = utils::RandomGenerator::getBool(Config::SPAWN_PROBABILITY_2) ? Config::TILE_VALUE_LOW : Config::TILE_VALUE_HIGH;

            const int r = emptyCells[index].first;
            const int c = emptyCells[index].second;

            grid_[r][c] = val;
            idGrid_[r][c] = nextId_++;
            lastSpawnPos_ = {r, c};
        }
    }

    /**
     * @brief Checks if the game is over.
     *
     * The game is over if there are no empty cells and no possible merges
     * either horizontally or vertically.
     *
     * @return True if the game is over, false otherwise.
     */
    bool Board::isGameOver() const {
        // Check for any empty cells. If one is found, the game is not over.
        for (const auto& row : grid_) {
            for (const Tile t : row) {
                if (t == 0) return false;
            }
        }
        // Check for possible merges. If any adjacent cells have the same value,
        // the game is not over.
        for (int i = 0; i < size_; ++i) {
            for (int j = 0; j < size_; ++j) {
                const Tile current = grid_[i][j];
                if (j < size_ - 1 && current == grid_[i][j + 1]) return false; // Check horizontal merge
                if (i < size_ - 1 && current == grid_[i + 1][j]) return false; // Check vertical merge
            }
        }
        // If no empty cells and no possible merges, the game is over.
        return true;
    }

    int Board::getScore() const { return score_; }

    int Board::getHighScore() const { return highScore_; }

    bool Board::hasWon() const { return hasReachedWinTile_; }
}  // namespace tfe::core
