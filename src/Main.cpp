#include "Game.h"

int main() {
    auto game = new GoL::Game({ .ruleString = GoL::Rules::LifeWithoutDeath });
    game->Run();
    delete game;

    return 0;
}
