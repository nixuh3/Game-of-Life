#include "game.hpp"

int main()
{
    auto game = new gol::Game({.ruleString = gol::Rules::Life});
    game->Run();
    delete game;

    return 0;
}
