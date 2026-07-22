#include "Game.h"
#include <raymath.h>
#include <unordered_map>
#include <array>
#include <iostream>
#include <cmath>
#include <cctype>

namespace GoL {

Game::Game(const GameSpecs& specs)
    : m_windowWidth(specs.windowWidth), m_windowHeight(specs.windowHeight), m_ruleString(specs.ruleString) {
    InitWindow(m_windowWidth, m_windowHeight, "Game of Life");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);

    Clear();
    if (!ParseRuleString()) {
        std::cout << "Invalid rule string\n";
        return;
    }

    m_camera.offset = { static_cast<float>(m_windowWidth) / 2.0f, static_cast<float>(m_windowHeight) / 2.0f };
    m_camera.target = { 0.0f, 0.0f };
    m_camera.rotation = 0.0f;
    m_camera.zoom = 1.0f;
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
    m_windowWidth = GetScreenWidth();
    m_windowHeight = GetScreenHeight();
    m_camera.offset = { static_cast<float>(m_windowWidth) / 2.0f, static_cast<float>(m_windowHeight) / 2.0f };

    // Mouse inputs
    Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), m_camera);
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        int mouseX = static_cast<int>(std::floor(mousePos.x / CELL_SIZE));
        int mouseY = static_cast<int>(std::floor(mousePos.y / CELL_SIZE));

        ToggleCell({ mouseX, mouseY });
    }
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
        Vector2 delta = GetMouseDelta();

        m_camera.target.x -= delta.x / m_camera.zoom;
        m_camera.target.y -= delta.y / m_camera.zoom;
    }
    float wheel = GetMouseWheelMove();
    if (wheel != 0.0f) {
        Vector2 mouseWorldBefore = GetScreenToWorld2D(GetMousePosition(), m_camera);

        m_camera.zoom = std::exp(std::log(m_camera.zoom) + wheel * 0.1f);
        m_camera.zoom = Clamp(m_camera.zoom, 0.1f, 3.0f);

        Vector2 mouseWorldAfter = GetScreenToWorld2D(GetMousePosition(), m_camera);

        m_camera.target.x += (mouseWorldBefore.x - mouseWorldAfter.x);
        m_camera.target.y += (mouseWorldBefore.y - mouseWorldAfter.y);
    }

    // Keyboard inputs
    m_camera.target.x += static_cast<float>(IsKeyDown(KEY_RIGHT) - IsKeyDown(KEY_LEFT)) * dt * CAMERA_SPEED;
    m_camera.target.y += static_cast<float>(IsKeyDown(KEY_DOWN) - IsKeyDown(KEY_UP)) * dt * CAMERA_SPEED;

    if (IsKeyPressed(KEY_X)) {
        Step();
    }
    if (IsKeyPressed(KEY_R)) {
        Randomize();
    }
    if (IsKeyPressed(KEY_SPACE)) {
        m_simulating ^= 1;
    }
    if (IsKeyPressed(KEY_C)) {
        Clear();
    }
    if (IsKeyPressed(KEY_W)) {
        m_updateInterval += 0.05f;
        m_updateInterval = Clamp(m_updateInterval, 0.05f, 2.0f);
        std::cout << "Update interval: " << m_updateInterval << "\n";
    } else if (IsKeyPressed(KEY_S)) {
        m_updateInterval -= 0.05f;
        m_updateInterval = Clamp(m_updateInterval, 0.05f, 2.0f);
        std::cout << "Update interval: " << m_updateInterval << "\n";
    }

    // Update timer
    m_timer += GetFrameTime();
    if (m_simulating && m_timer >= m_updateInterval) {
        Step();
        m_timer = 0.0f;
    }
}

void Game::Draw() const {
    BeginDrawing();
    ClearBackground(BLACK);

    BeginMode2D(m_camera);

    for (const auto& cell : m_activeCells) {
        Rectangle rect{ CELL_SIZE * static_cast<float>(cell.x), CELL_SIZE * static_cast<float>(cell.y),
            CELL_SIZE, CELL_SIZE };
        DrawRectangleRec(rect, GREEN);
    }

    EndMode2D();

    DrawFPS(10, 10);
    EndDrawing();
}

void Game::ToggleCell(const Vec2i& pos) {
    if (m_activeCells.contains(pos)) {
        m_activeCells.erase(pos);
    } else {
        m_activeCells.insert(pos);
    }
}

void Game::Clear() {
    m_simulating = false;
    m_activeCells.clear();
}

void Game::Randomize() {
    Clear();

    Vector2 topLeftWorld = GetScreenToWorld2D({ 0.0f, 0.0f }, m_camera);
    Vector2 bottomRightWorld = GetScreenToWorld2D(
        { static_cast<float>(m_windowWidth), static_cast<float>(m_windowHeight) }, m_camera);

    int minX = static_cast<int>(std::floor(topLeftWorld.x / CELL_SIZE));
    int maxX = static_cast<int>(std::ceil(bottomRightWorld.x / CELL_SIZE));
    int minY = static_cast<int>(std::floor(topLeftWorld.y / CELL_SIZE));
    int maxY = static_cast<int>(std::ceil(bottomRightWorld.y / CELL_SIZE));

    for (int x = minX; x <= maxX; ++x) {
        for (int y = minY; y <= maxY; ++y) {
            if (GetRandomValue(1, 3) == 1) {
                m_activeCells.insert({ x, y });
            }
        }
    }
}

void Game::Step() {
    std::unordered_map<Vec2i, int, Vec2i::Hash> neighborCounts;

    static std::array<Vec2i, 8> s_dirOffsets = {
        { { 0, 1 }, { 1, 0 }, { 0, -1 }, { -1, 0 }, { 1, 1 }, { -1, 1 }, { 1, -1 }, { -1, -1 } }
    };

    for (const auto& cell : m_activeCells) {
        for (const auto& [dx, dy] : s_dirOffsets) {
            neighborCounts[{ dx + cell.x, dy + cell.y }]++;
        }
    }

    Grid nextAliveCells;

    for (const auto& [pos, count] : neighborCounts) {
        bool isAlive = m_activeCells.contains(pos);

        if (isAlive && m_survival.contains(count)) {
            nextAliveCells.insert(pos); // Survival
        } else if (!isAlive && m_birth.contains(count)) {
            nextAliveCells.insert(pos); // Birth
        }
    }

    m_activeCells = std::move(nextAliveCells);
}

bool Game::ParseRuleString() {
    m_birth.clear();
    m_survival.clear();

    enum class ParseMode : uint8_t { NONE, BIRTH, SURVIVAL };
    ParseMode mode = ParseMode::NONE;

    for (char c : m_ruleString) {
        if (c == 'B' || c == 'b') {
            mode = ParseMode::BIRTH;
        } else if (c == 'S' || c == 's') {
            mode = ParseMode::SURVIVAL;
        } else if (std::isdigit(static_cast<unsigned char>(c))) {
            int digit = c - '0';
            if (mode == ParseMode::BIRTH) {
                m_birth.insert(digit);
            } else if (mode == ParseMode::SURVIVAL) {
                m_survival.insert(digit);
            }
        }
    }

    // Return true if both sets were populated (valid rule)
    return !m_birth.empty() && !m_survival.empty();
}

}
