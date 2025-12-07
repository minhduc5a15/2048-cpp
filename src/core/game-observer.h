#pragma once

#include "types.h"

namespace tfe {

    /**
     * @class IGameObserver
     * @brief An interface for classes that need to observe game events from the Board.
     *
     * This follows the Observer design pattern, where the Board is the "subject"
     * and classes that implement this interface are the "observers". This decouples
     * the core game logic from systems that react to it, like the UI or audio.
     */
    class IGameObserver {
    public:
        virtual ~IGameObserver() = default;

        /**
         * @brief Called when a new tile is spawned on the board.
         * @param r The row where the tile spawned.
         * @param c The column where the tile spawned.
         * @param value The value of the new tile (e.g., 2 or 4).
         */
        virtual void onTileSpawn(int r, int c, tfe::core::Tile value) = 0;

        /**
         * @brief Called when two tiles merge to form a new tile.
         * @param r The row where the merge occurred.
         * @param c The column where the merge occurred.
         * @param newValue The value of the newly formed tile (e.g., 8, 16, etc.).
         */
        virtual void onTileMerge(int r, int c, tfe::core::Tile newValue) = 0;

        /**
         * @brief Called when a tile moves from one position to another without merging.
         * @param fromR The starting row.
         * @param fromC The starting column.
         * @param toR The destination row.
         * @param toC The destination column.
         * @param value The value of the moving tile.
         */
        virtual void onTileMove(int fromR, int fromC, int toR, int toC, tfe::core::Tile value) = 0;

        /**
         * @brief Called when the game is over.
         */
        virtual void onGameOver() = 0;

        /**
         * @brief Called when the board is reset at the start of a new game.
         */
        virtual void onGameReset() = 0;
    };

} // namespace tfe