#pragma once

namespace tfe::core {

    /**
     * @interface IGameObserver
     * @brief Interface for observing game events (Observer Pattern).
     *
     * This interface allows different subsystems (like the GUI renderer, audio manager,
     * or AI trainer) to react to changes in the core game state without the core
     * engine needing to know about them.
     */
    class IGameObserver {
    public:
        virtual ~IGameObserver() = default;

        /**
         * @brief Called when the game is reset (new game starts).
         */
        virtual void onGameReset() {}

        /**
         * @brief Called when the game ends (no more moves).
         */
        virtual void onGameOver() {}

        /**
         * @brief Called when a new tile is spawned on the board.
         * @param r Row index.
         * @param c Column index.
         * @param value The value of the spawned tile (2 or 4).
         */
        virtual void onTileSpawn(int r, int c, int value) {}

        /**
         * @brief Called when a tile moves from one position to another.
         * @param fromR Source row.
         * @param fromC Source column.
         * @param toR Destination row.
         * @param toC Destination column.
         * @param value The value of the moving tile.
         */
        virtual void onTileMove(int fromR, int fromC, int toR, int toC, int value) {}

        /**
         * @brief Called when two tiles merge into a new value.
         * @param r Row index where the merge happened.
         * @param c Column index where the merge happened.
         * @param newValue The resulting value after merge.
         */
        virtual void onTileMerge(int r, int c, int newValue) {}
    };

}  // namespace tfe::core