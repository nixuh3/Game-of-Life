#pragma once

#include <raylib.h>
#include <unordered_set>
#include <cstdint>
#include <string>

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
    bool ParseRuleString();

    static constexpr float CELL_SIZE = 20.0f;
    int m_windowWidth;
    int m_windowHeight;

    Camera2D m_camera;
    static constexpr float CAMERA_SPEED = 600.0f;
    static constexpr float ZOOM_SPEED = 60.0f;

    using Grid = std::unordered_set<Vec2i, Vec2i::Hash>;
    Grid m_activeCells;

    std::string m_ruleString;
    std::unordered_set<int> m_birth;
    std::unordered_set<int> m_survival;

    bool m_simulating = false;
    float m_timer = 0.0f;
    float m_updateInterval = 0.1f;
};

}
