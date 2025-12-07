#include "random-generator.h"

namespace tfe::utils {

    std::mt19937& RandomGenerator::getEngine() {
        static std::random_device rd;
        static std::mt19937 engine(rd()); // Khởi tạo 1 lần duy nhất (static)
        return engine;
    }

    int RandomGenerator::getInt(int min, int max) {
        std::uniform_int_distribution<int> dist(min, max);
        return dist(getEngine());
    }

    bool RandomGenerator::getBool(double probability) {
        std::bernoulli_distribution dist(probability);
        return dist(getEngine());
    }

} // namespace tfe::utils