#include "random-generator.h"

namespace tfe::utils {

    std::mt19937& RandomGenerator::getEngine() {
        static std::random_device rd;
        static std::mt19937 engine(rd());
        return engine;
    }

    void RandomGenerator::seed(unsigned int seed) {
        getEngine().seed(seed);
    }

    int RandomGenerator::getInt(const int min, const int max) {
        std::uniform_int_distribution<int> dist(min, max);
        return dist(getEngine());
    }

    bool RandomGenerator::getBool(const double probability) {
        std::bernoulli_distribution dist(probability);
        return dist(getEngine());
    }

}  // namespace tfe::utils