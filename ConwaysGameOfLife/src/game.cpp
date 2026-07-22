#include "game.h"
#include <raymath.h>
#include <unordered_map>
#include <array>
#include <iostream>
#include <cmath>

namespace GoL {

Game::Game() {
    InitWindow(m_ScreenWidth, m_ScreenHeight, "Game of Life");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);

    Clear();

    m_Camera.offset = { static_cast<float>(m_ScreenWidth) / 2.0f, static_cast<float>(m_ScreenHeight) / 2.0f };
    m_Camera.target = { 0.0f, 0.0f };
    m_Camera.rotation = 0.0f;
    m_Camera.zoom = 1.0f;
}

Game::~Game() { CloseWindow(); }

void Game::Run() {
    while (!WindowShouldClose()) {
        Update(GetFrameTime());
        Draw();
    }
}

void Game::Update(float dt) {
    // Update screen dimensions
    m_ScreenWidth = GetScreenWidth();
    m_ScreenHeight = GetScreenHeight();
    m_Camera.offset = { static_cast<float>(m_ScreenWidth) / 2.0f, static_cast<float>(m_ScreenHeight) / 2.0f };

    // Mouse inputs
    Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), m_Camera);
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        int mouseX = static_cast<int>(std::floor(mousePos.x / CELL_SIZE));
        int mouseY = static_cast<int>(std::floor(mousePos.y / CELL_SIZE));

        ToggleCell({ mouseX, mouseY });
    }
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
        Vector2 delta = GetMouseDelta();

        m_Camera.target.x -= delta.x / m_Camera.zoom;
        m_Camera.target.y -= delta.y / m_Camera.zoom;
    }
    float wheel = GetMouseWheelMove();
    if (wheel != 0.0f) {
        Vector2 mouseWorldBefore = GetScreenToWorld2D(GetMousePosition(), m_Camera);

        m_Camera.zoom = std::exp(std::log(m_Camera.zoom) + wheel * 0.1f);
        m_Camera.zoom = Clamp(m_Camera.zoom, 0.1f, 3.0f);

        Vector2 mouseWorldAfter = GetScreenToWorld2D(GetMousePosition(), m_Camera);

        m_Camera.target.x += (mouseWorldBefore.x - mouseWorldAfter.x);
        m_Camera.target.y += (mouseWorldBefore.y - mouseWorldAfter.y);
    }

    // Keyboard inputs
    m_Camera.target.x += static_cast<float>(IsKeyDown(KEY_RIGHT) - IsKeyDown(KEY_LEFT)) * dt * CAMERA_SPEED;
    m_Camera.target.y += static_cast<float>(IsKeyDown(KEY_DOWN) - IsKeyDown(KEY_UP)) * dt * CAMERA_SPEED;

    if (IsKeyPressed(KEY_X)) {
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
    if (IsKeyPressed(KEY_W)) {
        m_UpdateInterval += 0.05f;
        m_UpdateInterval = Clamp(m_UpdateInterval, 0.05f, 2.0f);
        std::cout << "Update interval: " << m_UpdateInterval << "\n";
    } else if (IsKeyPressed(KEY_S)) {
        m_UpdateInterval -= 0.05f;
        m_UpdateInterval = Clamp(m_UpdateInterval, 0.05f, 2.0f);
        std::cout << "Update interval: " << m_UpdateInterval << "\n";
    }

    // Update timer
    m_Timer += GetFrameTime();
    if (m_Simulating && m_Timer >= m_UpdateInterval) {
        Step();
        m_Timer = 0.0f;
    }
}

void Game::Draw() const {
    BeginDrawing();
    ClearBackground(BLACK);

    BeginMode2D(m_Camera);

    for (const auto& cell : m_AliveCells) {
        Rectangle rect{ CELL_SIZE * static_cast<float>(cell.X), CELL_SIZE * static_cast<float>(cell.Y),
            CELL_SIZE, CELL_SIZE };
        DrawRectangleRec(rect, GREEN);
    }

    EndMode2D();

    DrawFPS(10, 10);
    EndDrawing();
}

void Game::ToggleCell(Vec2i pos) {
    if (m_AliveCells.contains(pos)) {
        m_AliveCells.erase(pos);
    } else {
        m_AliveCells.insert(pos);
    }
}

void Game::Clear() { m_AliveCells.clear(); }

// Only inside the viewport
void Game::Randomize() {
    Clear();

    Vector2 topLeftWorld = GetScreenToWorld2D({ 0.0f, 0.0f }, m_Camera);
    Vector2 bottomRightWorld = GetScreenToWorld2D(
        { static_cast<float>(m_ScreenWidth), static_cast<float>(m_ScreenHeight) }, m_Camera);

    int minX = static_cast<int>(std::floor(topLeftWorld.x / CELL_SIZE));
    int maxX = static_cast<int>(std::ceil(bottomRightWorld.x / CELL_SIZE));
    int minY = static_cast<int>(std::floor(topLeftWorld.y / CELL_SIZE));
    int maxY = static_cast<int>(std::ceil(bottomRightWorld.y / CELL_SIZE));

    for (int x = minX; x <= maxX; ++x) {
        for (int y = minY; y <= maxY; ++y) {
            if (GetRandomValue(1, 3) == 1) {
                m_AliveCells.insert({ x, y });
            }
        }
    }
}

void Game::Step() {
    std::unordered_map<Vec2i, int, Vec2iHash> neighborCounts;

    static std::array<Vec2i, 8> s_dirOffsets = {
        { { 0, 1 }, { 1, 0 }, { 0, -1 }, { -1, 0 }, { 1, 1 }, { -1, 1 }, { 1, -1 }, { -1, -1 } }
    };

    for (const auto& cell : m_AliveCells) {
        for (const auto& [dx, dy] : s_dirOffsets) {
            neighborCounts[{ dx + cell.X, dy + cell.Y }]++;
        }
    }

    std::unordered_set<Vec2i, Vec2iHash> nextAliveCells;

    for (const auto& [pos, count] : neighborCounts) {
        bool isAlive = m_AliveCells.contains(pos);

        if (isAlive && (count == 2 || count == 3)) {
            nextAliveCells.insert(pos); // Survival
        } else if (!isAlive && count == 3) {
            nextAliveCells.insert(pos); // Birth
        }
    }

    m_AliveCells = std::move(nextAliveCells);
}

}
