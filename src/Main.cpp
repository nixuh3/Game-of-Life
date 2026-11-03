#include "game.hpp"

int main() {
    auto game = new gol::Game({ .ruleString = gol::Rules::Anneal });
    game->Run();
    delete game;

    return 0;
}
