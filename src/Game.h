#pragma once

#include "CameraController.h"
#include "Chunk.h"
#include "Rules.h"
#include <SFML/Graphics.hpp>
#include <random>
#include <string_view>

namespace GoL {

struct GameSpecs {
    std::string_view ruleString = Rules::Life;
    sf::Vector2u windowSize     = {1920, 1080};
};

class Game {
  public:
    Game(const GameSpecs& specs = {});
    ~Game() = default;

    Game(const Game&)                = delete;
    Game& operator=(const Game&)     = delete;
    Game(Game&&) noexcept            = default;
    Game& operator=(Game&&) noexcept = default;

    void Run();

  private:
    enum CellType : uint8_t { ALIVE, DEAD };
    constexpr friend CellType operator~(CellType type) { return type == ALIVE ? DEAD : ALIVE; }

    void Update(float dt);
    void Draw(float dt);
    void ToggleCell(sf::Vector2i pos);
    void Clear();
    void Randomize();

    static constexpr float CELL_SIZE        = 20.0f;
    static constexpr float CHUNK_WORLD_SIZE = 64.0f * CELL_SIZE;

    sf::RenderWindow m_window;
    sf::Vector2u m_windowSize;
    sf::Clock m_clock;
    sf::Font m_font;

    CameraController m_camera;

    ChunkMap m_chunks;
    ChunkMap m_nextChunks;

    std::string_view m_ruleString;
    RuleSet m_rules;

    sf::Vector2i m_mouseTarget = {};
    bool m_simulating          = false;
    bool m_showDebugInfo       = false;
    float m_timer              = 0.0f;
    float m_updateInterval     = 0.1f;

    std::mt19937 m_gen;
};

} // namespace GoL
