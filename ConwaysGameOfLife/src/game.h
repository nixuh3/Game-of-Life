#pragma once

#include <raylib.h>
#include <array>
#include <cstdint>

class Game {
  public:
    Game();
    ~Game();

    void Run();

  private:
    enum CellType : uint8_t { ALIVE, DEAD };

    friend CellType operator~(CellType type) {
        return type == ALIVE ? DEAD : ALIVE;
    }

    void Update();
    void Draw() const;

    void Clear();
    void Randomize();
    void Step();

    static constexpr int WINDOW_WIDTH = 1920;
    static constexpr int WINDOW_HEIGHT = 1080;
    static constexpr float CELL_SIZE = 20.0f;
    static constexpr int CELL_NB_ROW = static_cast<int>(WINDOW_HEIGHT / CELL_SIZE);
    static constexpr int CELL_NB_COL = static_cast<int>(WINDOW_WIDTH / CELL_SIZE);

    std::array<std::array<CellType, CELL_NB_COL>, CELL_NB_ROW> m_Grid;

    bool m_Simulating = false;
    float m_Timer = 0.0f;
    float m_UpdateInterval = 0.1f;
};