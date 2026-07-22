#include "Game.h"

int main() {
    auto game = new GoL::Game({ .ruleString = "B3/S012345678" });
    game->Run();
    delete game;

    return 0;
}
