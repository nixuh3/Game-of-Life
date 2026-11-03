#pragma once

#include "rules.hpp"
#include "camera.hpp"
#include "chunk.hpp"
#include <string_view>
#include <memory>

namespace gol {

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
    Game(Game&&) noexcept = default;
    Game& operator=(Game&&) noexcept = default;

    void Run();

  private:
    static constexpr float CELL_SIZE = 20.0f;
    static constexpr float CHUNK_WORLD_SIZE = 64.0f * CELL_SIZE;

    int m_windowWidth;
    int m_windowHeight;

    CameraController m_camera;

    ChunkMap m_chunks;
    ChunkMap m_nextChunks;

    std::string_view m_ruleString;
    RuleSet m_rules;

    std::unordered_map<Vec2i, int, Vec2i::Hash> m_neighborCounts;

    Vec2i m_mouseTarget = {};
    bool m_simulating = false;
    bool m_showDebugInfo = false;
    float m_timer = 0.0f;
    float m_updateInterval = 0.1f;

    void Update(float dt);
    void Draw();
    void ToggleCell(const Vec2i& pos);
    void Clear();
    void Randomize();
};

}
