#include "random-generator.h"

namespace tfe::utils {

    /**
     * @brief Accesses the singleton Mersenne Twister random number generator.
     * 
     * Uses a static local variable to ensure:
     * 1. The engine is initialized only once (lazy initialization).
     * 2. It is thread-safe (in C++11 and later).
     * 3. It is properly seeded using std::random_device.
     * 
     * @return Reference to the std::mt19937 engine.
     */
    std::mt19937& RandomGenerator::getEngine() {
        static std::random_device rd;  // Non-deterministic source (hardware entropy)
        static std::mt19937 engine(rd()); // Seed the PRNG
        return engine;
    }

    /**
     * @brief Generates a random integer within a range.
     * @param min Inclusive minimum value.
     * @param max Inclusive maximum value.
     * @return Random integer in [min, max].
     */
    int RandomGenerator::getInt(const int min, const int max) {
        std::uniform_int_distribution<int> dist(min, max);
        return dist(getEngine());
    }

    /**
     * @brief Generates a boolean value (true/false) based on a probability.
     * @param probability The chance of returning true (0.0 to 1.0).
     * @return True or False.
     */
    bool RandomGenerator::getBool(const double probability) {
        std::bernoulli_distribution dist(probability);
        return dist(getEngine());
    }

}  // namespace tfe::utils