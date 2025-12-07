#pragma once
#include <random>

namespace tfe::utils { // tfe = twenty-four-eight

    class RandomGenerator {
    public:
        // Trả về số nguyên ngẫu nhiên trong đoạn [min, max]
        static int getInt(int min, int max);

        // Trả về true với xác suất probability (0.0 - 1.0)
        static bool getBool(double probability);

    private:
        static std::mt19937& getEngine();
    };

} // namespace tfe::utils