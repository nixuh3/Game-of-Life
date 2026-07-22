#include "game.h"
#include <raymath.h>
#include <iostream>

Game::Game() {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Game of Life");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);

    Clear();
}

Game::~Game() { CloseWindow(); }

void Game::Run() {
    while (!WindowShouldClose()) {
        Update();

        BeginDrawing();
        ClearBackground(WHITE);

        Draw();

        DrawFPS(10, 10);
        EndDrawing();
    }
}

void Game::Update() {
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 mousePos = GetMousePosition();

        int mouseR = static_cast<int>(mousePos.y / CELL_SIZE);
        int mouseC = static_cast<int>(mousePos.x / CELL_SIZE);

        if (mouseR >= 0 && mouseR < CELL_NB_ROW && mouseC >= 0 && mouseC < CELL_NB_COL) {
            m_Grid[mouseR][mouseC] = ~m_Grid[mouseR][mouseC];
        }
    }

    if (IsKeyPressed(KEY_S)) {
        Step();
    }
    if (IsKeyPressed(KEY_R)) {
        Randomize();
    }
    if (IsKeyPressed(KEY_SPACE)) {
        m_Simulating ^= 1;
    }
    if (IsKeyPressed(KEY_C)) {
        Clear();
    }
    if (IsKeyPressed(KEY_UP)) {
        m_UpdateInterval += 0.05f;
        Clamp(m_UpdateInterval, 0.05f, 2.0f);
        std::cout << "Update interval: " << m_UpdateInterval << "\n";
    } else if (IsKeyPressed(KEY_DOWN)) {
        m_UpdateInterval -= 0.05f;
        Clamp(m_UpdateInterval, 0.05f, 2.0f);
        std::cout << "Update interval: " << m_UpdateInterval << "\n";
    }

    m_Timer += GetFrameTime();
    if (m_Simulating && m_Timer >= m_UpdateInterval) {
        Step();
        m_Timer = 0.0f;
    }
}

void Game::Draw() const {
    for (int row = 0; row < CELL_NB_ROW; ++row) {
        for (int col = 0; col < CELL_NB_COL; ++col) {
            Rectangle rect{ CELL_SIZE * col, CELL_SIZE * row, CELL_SIZE, CELL_SIZE };
            DrawRectangleRec(rect, m_Grid[row][col] ? BLACK : GREEN);
        }
    }
}

void Game::Clear() {
    for (int row = 0; row < CELL_NB_ROW; ++row) {
        for (int col = 0; col < CELL_NB_COL; ++col) {
            m_Grid[row][col] = DEAD;
        }
    }
}

void Game::Randomize() {
    for (auto& row : m_Grid) {
        for (auto& cell : row) {
            cell = GetRandomValue(0, 1) == 0 ? DEAD : ALIVE;
        }
    }
}

void Game::Step() {
    auto newGrid = m_Grid;

    static std::array<std::pair<int, int>, 8> s_dirOffsets = {
        { { 0, 1 }, { 1, 0 }, { 0, -1 }, { -1, 0 }, { 1, 1 }, { -1, 1 }, { 1, -1 }, { -1, -1 } }
    };

    for (int row = 0; row < CELL_NB_ROW; ++row) {
        for (int col = 0; col < CELL_NB_COL; ++col) {
            auto& cell = newGrid[row][col];

            int aliveNeighbors = 0;

            for (auto [dr, dc] : s_dirOffsets) {
                int nr = dr + row;
                int nc = dc + col;

                if (nr < 0 || nr >= CELL_NB_ROW || nc < 0 || nc >= CELL_NB_COL) {
                    break;
                }

                if (m_Grid[nr][nc] == ALIVE) {
                    aliveNeighbors++;
                }
            }

            if (aliveNeighbors < 2 || aliveNeighbors > 3) {
                cell = DEAD;
            } else if (aliveNeighbors == 3) {
                cell = ALIVE;
            }
        }
    }

    m_Grid = newGrid;
}
