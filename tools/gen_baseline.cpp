#include "core/board.h"
#include "utils/random-generator.h"
#include <iostream>
#include <fstream>
#include <random>

int main(int argc, char** argv) {
    tfe::utils::RandomGenerator::seed(12345);
    
    // Separate RNG for moves to ensure the input sequence is deterministic
    std::mt19937 moveRng(54321);
    std::uniform_int_distribution<int> dist(0, 3);

    tfe::core::Board board;
    
    uint64_t rollingHash = 0;
    
    for (int i = 0; i < 100000; ++i) {
        int dirInt = dist(moveRng);
        tfe::core::Direction dir = static_cast<tfe::core::Direction>(dirInt);
        
        board.move(dir);
        
        if (board.isGameOver()) {
            board.reset();
        }
        
        tfe::core::GameState state = board.getState();
        // Simple hash combining board and score
        rollingHash ^= state.board;
        rollingHash += state.score;
        // Rotate left 5 bits
        rollingHash = (rollingHash << 5) | (rollingHash >> 59);
    }

    std::cout << "Baseline Hash: " << rollingHash << std::endl;
    std::cout << "Final Score: " << board.getScore() << std::endl;
    
    std::ofstream out("baseline_moves.bin");
    out << rollingHash << "\n" << board.getScore() << "\n";
    out.close();
    
    return 0;
}
