#pragma once
#pragma once

#include <raylib.h>
#include <unordered_set>
#include <cstdint>

namespace GoL {

class Game {
  public:
    Game();
    ~Game();

    void Run();

  private:
    struct Vec2i {
      public:
        int X;
        int Y;

        bool operator==(const Vec2i& other) const { return X == other.X && Y == other.Y; }
    };

    struct Vec2iHash {
        std::size_t operator()(const Vec2i& p) const {
            return std::hash<int64_t>()(
                (static_cast<int64_t>(p.X) << 32) ^ static_cast<unsigned int>(p.Y));
        }
    };

    enum CellType : uint8_t { ALIVE, DEAD };

    friend CellType operator~(CellType type) { return type == ALIVE ? DEAD : ALIVE; }

    void Update(float dt);
    void Draw() const;

    void ToggleCell(Vec2i pos);
    void Clear();
    void Randomize();
    void Step();

    static constexpr float CELL_SIZE = 20.0f;
    int m_ScreenWidth = 1920;
    int m_ScreenHeight = 1080;

    Camera2D m_Camera;
    static constexpr float CAMERA_SPEED = 600.0f;
    static constexpr float ZOOM_SPEED = 60.0f;

    std::unordered_set<Vec2i, Vec2iHash> m_AliveCells;

    bool m_Simulating = false;
    float m_Timer = 0.0f;
    float m_UpdateInterval = 0.1f;
};

}
