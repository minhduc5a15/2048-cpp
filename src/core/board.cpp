#include "board.h"

#include <array>
#include <vector>

#include "bitboard_ops.h"
#include "config.h"
#include "lookup_table.h"
#include "score/score-manager.h"
#include "utils/random-generator.h"

namespace tfe::core {

    namespace {
        constexpr int BITS_PER_ROW = 16;
        constexpr int BITS_PER_TILE = 4;
        constexpr int TILES_PER_ROW = 4;
        constexpr Bitboard TILE_MASK = 0xF;
    }  // namespace

    Board::Board() {
        static bool tableInitialized = false;
        if (!tableInitialized) {
            LookupTable::init();
            tableInitialized = true;
        }
        highScore_ = tfe::score::ScoreManager::load_high_score();
        reset();
    }

    void Board::reset() {
        board_ = 0;
        score_ = 0;
        hasReachedWinTile_ = false;
        notifyGameReset();
        spawnRandomTile();
        spawnRandomTile();
    }

    Grid Board::getGrid() const {
        Grid result(4, std::vector<int>(4));
        for (int r = 0; r < 4; ++r) {
            for (int c = 0; c < 4; ++c) {
                const Tile t = getTile(r, c);
                result[r][c] = (t == 0) ? 0 : (1 << t);
            }
        }
        return result;
    }

    Tile Board::getTile(const int row, const int col) const { return (board_ >> ((row * BITS_PER_ROW) + (col * BITS_PER_TILE))) & TILE_MASK; }

    void Board::setTile(const int row, const int col, const Tile value) {
        const int shift = (row * BITS_PER_ROW) + (col * BITS_PER_TILE);
        board_ &= ~(static_cast<Bitboard>(TILE_MASK) << shift);
        board_ |= (static_cast<Bitboard>(value) << shift);
    }

    void Board::transpose() { board_ = BitboardOps::transpose64(board_); }

    bool Board::move(const Direction dir) {
        // 1. Calculate the final state using the optimized BitboardOps (Source of Truth).
        // This ensures the game logic remains absolutely correct and efficient.
        auto [newBoard, moveScore] = BitboardOps::executeMove(board_, dir);

        // If the board hasn't changed, the move is invalid.
        if (newBoard == board_) {
            return false;
        }

        // 2. Simulate the move tile-by-tile to generate Animation Events.
        // BitboardOps gives us the result, but not the "path" of each tile.
        // We reconstruct the path here to notify the GUI.

        // Helper struct to track the simulation state of a single line
        struct MergedTile {
            Tile val;     // Current Exponent
            bool merged;  // Has this tile already merged in this step?
        };

        for (int line = 0; line < 4; ++line) {
            std::vector<MergedTile> virtualLine;
            virtualLine.reserve(4);

            // Iterate through positions in the line (0..3).
            // "pos" implies the distance from the wall we are moving towards.
            // pos=0 is the wall. pos=3 is the farthest tile.
            for (int pos = 0; pos < 4; ++pos) {
                int r, c;

                // Map abstract (line, pos) to physical (r, c) based on Direction
                switch (dir) {
                    case Direction::Left:
                        r = line;
                        c = pos;
                        break;
                    case Direction::Right:
                        r = line;
                        c = 3 - pos;
                        break;
                    case Direction::Up:
                        r = pos;
                        c = line;
                        break;
                    case Direction::Down:
                        r = 3 - pos;
                        c = line;
                        break;
                    default:
                        r = 0;
                        c = 0;
                        break;  // Should not happen
                }

                const Tile currentExp = getTile(r, c);
                if (currentExp == 0) continue;

                // Move/Merge Logic
                bool merged = false;
                if (!virtualLine.empty()) {
                    auto& last = virtualLine.back();
                    if (last.val == currentExp && !last.merged) {
                        // MERGE
                        last.val++;  // Increment exponent (2^k -> 2^(k+1))
                        last.merged = true;
                        merged = true;

                        // Calculate Destination Coordinates
                        const int destIdx = static_cast<int>(virtualLine.size()) - 1;
                        int destR, destC;
                        switch (dir) {
                            case Direction::Left:
                                destR = line;
                                destC = destIdx;
                                break;
                            case Direction::Right:
                                destR = line;
                                destC = 3 - destIdx;
                                break;
                            case Direction::Up:
                                destR = destIdx;
                                destC = line;
                                break;
                            case Direction::Down:
                                destR = 3 - destIdx;
                                destC = line;
                                break;
                            default:
                                destR = 0;
                                destC = 0;
                                break;
                        }

                                                // Notify: Tile moved from (r,c) to (destR, destC) AND then merged

                                                // FIX: Send raw exponent, remove (1 << ...) to prevent overflow

                                                notifyTileMove(r, c, destR, destC, currentExp);

                                                notifyTileMerge(destR, destC, last.val);

                                            }

                                        }

                        

                                        if (!merged) {

                                            // SLIDE (No Merge)

                                            virtualLine.push_back({currentExp, false});

                                            

                                            int destIdx = static_cast<int>(virtualLine.size()) - 1;

                                            

                                            // Only notify if the tile actually changed coordinates

                                            if (destIdx != pos) {

                                                int destR, destC;

                                                switch (dir) {

                                                    case Direction::Left:  destR = line; destC = destIdx; break;

                                                    case Direction::Right: destR = line; destC = 3 - destIdx; break;

                                                    case Direction::Up:    destR = destIdx; destC = line; break;

                                                    case Direction::Down:  destR = 3 - destIdx; destC = line; break;

                                                    default: destR = 0; destC = 0; break;

                                                }

                                                

                                                // FIX: Send raw exponent

                                                notifyTileMove(r, c, destR, destC, currentExp);

                                            }

                                        }
            }
        }

        // 3. Commit the state
        board_ = newBoard;
        score_ += moveScore;
        if (score_ > highScore_) highScore_ = score_;

        spawnRandomTile();
        return true;
    }

    void Board::spawnRandomTile() {
        std::array<int, BITS_PER_ROW> empty{};
        int count = 0;
        for (int i = 0; i < BITS_PER_ROW; ++i) {
            if (((board_ >> (i * BITS_PER_TILE)) & TILE_MASK) == 0) {
                empty[count++] = i;
            }
        }
        if (count > 0) {
            const int idx = empty[tfe::utils::RandomGenerator::getInt(0, count - 1)];
            const Tile val =
                tfe::utils::RandomGenerator::getBool(Config::SPAWN_PROBABILITY_2) ? Config::TILE_EXPONENT_LOW : Config::TILE_EXPONENT_HIGH;

            board_ |= (static_cast<Bitboard>(val) << (idx * BITS_PER_TILE));

            const int r = idx / TILES_PER_ROW;
            const int c = idx % TILES_PER_ROW;
            notifyTileSpawn(r, c, (1 << val));
        }
    }

    bool Board::isGameOver() const {
        if (BitboardOps::executeMove(board_, Direction::Left).first != board_) return false;
        if (BitboardOps::executeMove(board_, Direction::Right).first != board_) return false;
        if (BitboardOps::executeMove(board_, Direction::Up).first != board_) return false;
        if (BitboardOps::executeMove(board_, Direction::Down).first != board_) return false;

        notifyGameOver();
        return true;
    }

    GameState Board::getState() const { return GameState{board_, score_}; }

    void Board::loadState(const GameState& state) {
        board_ = state.board;
        score_ = state.score;
        notifyGameReset();
    }

    void Board::addObserver(IGameObserver* observer) { observers_.push_back(observer); }
    void Board::removeObserver(IGameObserver* observer) { std::erase(observers_, observer); }
    void Board::notifyGameReset() const {
        for (auto* o : observers_) o->onGameReset();
    }
    void Board::notifyGameOver() const {
        for (auto* o : observers_) o->onGameOver();
    }
    void Board::notifyTileSpawn(const int r, const int c, const int value) const {
        for (auto* o : observers_) o->onTileSpawn(r, c, value);
    }
    void Board::notifyTileMove(const int fromR, const int fromC, const int toR, const int toC, const Tile value) const {
        for (auto* o : observers_) o->onTileMove(fromR, fromC, toR, toC, value);
    }
    void Board::notifyTileMerge(const int r, const int c, const Tile newValue) const {
        for (auto* o : observers_) o->onTileMerge(r, c, newValue);
    }
}  // namespace tfe::core