#include "core/bitboard_ops.h"
#include "core/lookup_table.h"
#include <iostream>
#include <chrono>

int main(int argc, char** argv) {
    tfe::core::LookupTable::init();
    
    tfe::core::Bitboard b = 0x123456789ABCDEF0;
    long iters = 100000000; // 100M iters
    
    auto start = std::chrono::high_resolution_clock::now();
    
    long long dummy = 0;
    for(long i=0; i<iters; ++i) {
        // Alternating directions to prevent trivial steady state
        tfe::core::Direction dir = (i % 2 == 0) ? tfe::core::Direction::Left : tfe::core::Direction::Up;
        auto [nb, score] = tfe::core::BitboardOps::executeMove(b, dir);
        dummy += score;
        b = nb ^ (i & 0xFFFF); // Perturb board to keep it changing
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;
    
    std::cout << "Time: " << diff.count() << "s" << std::endl;
    std::cout << "OPS_PER_SEC: " << (double)iters / diff.count() << std::endl;
    std::cout << "Dummy: " << dummy << std::endl;
    
    return 0;
}
