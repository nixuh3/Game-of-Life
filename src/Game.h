#pragma once

#include <string_view>
#include <memory>

namespace GoL {

namespace Rules {

// From https://en.wikipedia.org/wiki/Life-like_cellular_automaton
static constexpr std::string_view Life = "B3/S23";
static constexpr std::string_view HighLife = "B36/S23";
static constexpr std::string_view LifeWithoutDeath = "B3/S012345678";
static constexpr std::string_view Seeds = "B2/S";
static constexpr std::string_view Replicator = "B1357/S1357";
static constexpr std::string_view Diamoeba = "B35678/S5678";
static constexpr std::string_view TwoxTwo = "B36/S125";
static constexpr std::string_view DayAndNight = "B3678/S34678";
static constexpr std::string_view Morley = "B368/S245";
static constexpr std::string_view Anneal = "B4678/S35678";

}

struct GameSpecs {
    std::string_view ruleString = Rules::Life;
    int windowWidth = 1920;
    int windowHeight = 1080;
};

class Game {
  public:
    Game(const GameSpecs& specs = {});
    ~Game();

    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;

    Game(Game&&) noexcept;
    Game& operator=(Game&&) noexcept;

    void Run();

  private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

}
