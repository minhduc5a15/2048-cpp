#include "trainer.h"

/**
 * @file main_train.cpp
 * @brief Entry point for the AI Training application.
 *
 * This executable is separate from the main game. It runs the Reinforcement Learning
 * loop to train the N-Tuple Network weights. The resulting weights are saved to
 * `build/bin/tuple_weights.bin` and can be loaded by the game AI.
 * 
 * Usage: ./2048-train [episodes]
 */

int main(int argc, char* argv[]) {
    // Default to 1 million episodes if not specified
    int episodes = 1000000;
    
    if (argc > 1) {
        episodes = std::stoi(argv[1]);
    }
    
    // Start training with an initial learning rate of 0.0001
    tfe::train::Trainer trainer(0.0001f);
    trainer.run(episodes);

    return 0;
}