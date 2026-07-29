#include "Game.h"

// Conway's Game of Life: B3/S23
// Life without death: B3/S012345678
// Seeds: B2/S
// Replicator: B1357/S1357
// Flock: B1/S12

int main() {
    auto game = new GoL::Game({ .ruleString = "B1/S12" });
    game->Run();
    delete game;

    return 0;
}
