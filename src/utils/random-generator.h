#pragma once
#include <random>

namespace tfe::utils {  // tfe = twenty-four-eight

    // A static utility class for generating random numbers.
    class RandomGenerator {
    public:
        // Seeds the random number generator for deterministic behavior.
        static void seed(unsigned int seed);

        // Returns a random integer in the inclusive range [min, max].
        static int getInt(int min, int max);

        // Returns true with a given probability (from 0.0 to 1.0).
        static bool getBool(double probability);

    private:
        // Provides access to the singleton random number engine.
        static std::mt19937& getEngine();
    };

}  // namespace tfe::utils