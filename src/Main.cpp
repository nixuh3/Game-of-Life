#include "Game.h"

int main() {
    auto game = new GoL::Game({.ruleString = GoL::Rules::TwoxTwo});
    game->Run();
    delete game;

    return 0;
}
