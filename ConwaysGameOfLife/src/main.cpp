#include "game.h"

int main() {
    auto game = new GoL::Game;
    game->Run();
    delete game;

    return 0;
}
