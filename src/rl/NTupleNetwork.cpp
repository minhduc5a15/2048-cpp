#include "NTupleNetwork.h"
#include <fstream>
#include <iostream>
#include <cmath>
#include <stdexcept>
#include <algorithm>

namespace tfe::rl {

    NTupleNetwork::NTupleNetwork() {
        // Hardcoded baseline tuple set as per specification
        // Rows
        addTuple({0, 1, 2, 3});
        addTuple({4, 5, 6, 7});
        addTuple({8, 9, 10, 11});
        addTuple({12, 13, 14, 15});

        // Cols
        addTuple({0, 4, 8, 12});
        addTuple({1, 5, 9, 13});
        addTuple({2, 6, 10, 14});
        addTuple({3, 7, 11, 15});

        // 2x2 Blocks
        addTuple({0, 1, 4, 5});
        addTuple({2, 3, 6, 7});
        addTuple({8, 9, 12, 13});
        addTuple({10, 11, 14, 15});

        // Snakes (Reversed rows)
        addTuple({3, 2, 1, 0});
        addTuple({15, 14, 13, 12});
    }

    void NTupleNetwork::addTuple(const std::vector<int>& cells) {
        tuples_.push_back(cells);
        // Size is 16^len. Since cells.size() is usually 4, this is 65536.
        // We use size_t for count.
        size_t weightCount = 1;
        for (size_t i = 0; i < cells.size(); ++i) {
            weightCount *= 16;
        }
        weights_.emplace_back(weightCount, 0.0f);
    }

    size_t NTupleNetwork::computeIndex(uint64_t board, size_t tupleId) const {
        if (tupleId >= tuples_.size()) {
            return 0; // Should throw or handle error
        }

        const auto& cells = tuples_[tupleId];
        size_t index = 0;
        size_t multiplier = 1;

        for (int cellIdx : cells) {
            // Extract 4-bit value at cellIdx
            // Board structure: 16 nibbles. Cell 0 is lowest 4 bits? 
            // Usually Cell 0 is Row 0 Col 0. Board storage depends on implementation.
            // Assuming tfe::core::Bitboard layout:
            // (board >> (cellIdx * 4)) & 0xF
            uint64_t val = (board >> (cellIdx * 4)) & 0xF;
            
            index += val * multiplier;
            multiplier *= 16;
        }
        return index;
    }

    float NTupleNetwork::evaluate(uint64_t board) const {
        float total = 0.0f;
        for (size_t i = 0; i < tuples_.size(); ++i) {
            size_t idx = computeIndex(board, i);
            total += weights_[i][idx];
        }
        return total;
    }

    void NTupleNetwork::updateWeightsForIndex(size_t tupleId, size_t index, float delta) {
        if (tupleId < weights_.size() && index < weights_[tupleId].size()) {
            weights_[tupleId][index] += delta;
        }
    }

    void NTupleNetwork::save(const std::string& path) const {
        std::ofstream out(path, std::ios::binary);
        if (!out) {
            throw std::runtime_error("Cannot open file for writing: " + path);
        }

        // Header
        const char magic[] = "NTUPLEv1\n";
        out.write(magic, sizeof(magic) - 1); // Don't write null terminator if string literal logic used carefully, 
                                             // usually sizeof includes null. "NTUPLEv1\n" is 9 chars.
                                             // Requirement: "NTUPLEv1\n"
        
        uint32_t numTuples = static_cast<uint32_t>(tuples_.size());
        out.write(reinterpret_cast<const char*>(&numTuples), sizeof(numTuples));

        for (size_t i = 0; i < tuples_.size(); ++i) {
            const auto& tuple = tuples_[i];
            uint8_t len = static_cast<uint8_t>(tuple.size());
            out.write(reinterpret_cast<const char*>(&len), sizeof(len));

            for (int cell : tuple) {
                uint8_t c = static_cast<uint8_t>(cell);
                out.write(reinterpret_cast<const char*>(&c), sizeof(c));
            }

            uint64_t wCount = static_cast<uint64_t>(weights_[i].size());
            out.write(reinterpret_cast<const char*>(&wCount), sizeof(wCount));

            out.write(reinterpret_cast<const char*>(weights_[i].data()), wCount * sizeof(float));
        }
        out.close();
    }

    void NTupleNetwork::load(const std::string& path) {
        std::ifstream in(path, std::ios::binary);
        if (!in) {
            throw std::runtime_error("Cannot open file for reading: " + path);
        }

        char magic[9];
        in.read(magic, 9);
        if (std::string(magic, 9) != "NTUPLEv1\n") {
            throw std::runtime_error("Invalid file format (Magic mismatch)");
        }

        uint32_t numTuples;
        in.read(reinterpret_cast<char*>(&numTuples), sizeof(numTuples));

        // Clear existing to match file? Or validate?
        // Requirement says "load". Usually overwrites or fills.
        // If hardcoded tuples are enforced, we should validate that file tuples match hardcoded ones?
        // Or simply reconstruct from file. 
        // Given constraint: "Hardcode these tuples as initial set". 
        // We will clear and reload to ensure we match the file state exactly.
        tuples_.clear();
        weights_.clear();

        for (uint32_t i = 0; i < numTuples; ++i) {
            uint8_t len;
            in.read(reinterpret_cast<char*>(&len), sizeof(len));

            std::vector<int> cells;
            cells.reserve(len);
            for (int k = 0; k < len; ++k) {
                uint8_t c;
                in.read(reinterpret_cast<char*>(&c), sizeof(c));
                cells.push_back(c);
            }
            tuples_.push_back(cells);

            uint64_t wCount;
            in.read(reinterpret_cast<char*>(&wCount), sizeof(wCount));

            std::vector<float> w(wCount);
            in.read(reinterpret_cast<char*>(w.data()), wCount * sizeof(float));
            weights_.push_back(w);
        }
        in.close();
    }

} // namespace tfe::rl
