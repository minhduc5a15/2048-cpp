#include "trainer.h"

int main(int argc, char* argv[]) {
    int episodes = 1000000;
    if (argc > 1) {
        episodes = std::stoi(argv[1]);
    }
    tfe::train::Trainer trainer(0.0025f);
    trainer.run(episodes);

    return 0;
}