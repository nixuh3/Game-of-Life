#pragma once

#include <raylib.h>
#include <unordered_set>
#include <cstdint>
#include <string>

#include "Chunk.h"
#include "Math.h"

namespace GoL {

struct GameSpecs {
    std::string ruleString = "B3/S23";
    int windowWidth = 1920;
    int windowHeight = 1080;
};

class Game {
  public:
    Game(const GameSpecs& specs = {});
    ~Game();

    void Run();

  private:
    enum CellType : uint8_t { ALIVE, DEAD };

    friend CellType operator~(CellType type) { return type == ALIVE ? DEAD : ALIVE; }

    void Update(float dt);
    void Draw() const;

    void ToggleCell(const Vec2i& pos);
    void Clear();
    void Randomize(); // Only inside the viewport
    void Step();
    void ParseRuleString();

    static constexpr float CELL_SIZE = 20.0f;
    static constexpr float CHUNK_WORLD_SIZE = 64.0f * CELL_SIZE;
    int m_windowWidth;
    int m_windowHeight;

    Camera2D m_camera;
    static constexpr float CAMERA_SPEED = 600.0f;
    static constexpr float ZOOM_SPEED = 60.0f;

    ChunkMap m_chunks;
    ChunkMap m_nextChunks;

    std::string m_ruleString;
    uint16_t m_birthMask = 0;
    uint16_t m_survivalMask = 0;

    std::unordered_map<Vec2i, int, Vec2i::Hash> m_neighborCounts;
    bool m_simulating = false;
    float m_timer = 0.0f;
    float m_updateInterval = 0.1f;
};

}
