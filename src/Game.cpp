#include "Game.h"
#include "Chunk.h"
#include "Math.h"
#include "Rules.h"
#include "Simulation.h"
#include "CameraController.h"
#include <raymath.h>

namespace GoL {

enum CellType : uint8_t { ALIVE, DEAD };

constexpr CellType operator~(CellType type) { return type == ALIVE ? DEAD : ALIVE; }

struct Game::Impl {
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

    explicit Impl(const GameSpecs& specs);
    ~Impl();

    void Update(float dt);
    void Draw();
    void ToggleCell(const Vec2i& pos);
    void Clear();
    void Randomize();
};

Game::Impl::Impl(const GameSpecs& specs)
    : m_windowWidth(specs.windowWidth),
      m_windowHeight(specs.windowHeight),
      m_ruleString(specs.ruleString),
      m_camera(m_windowWidth, m_windowHeight) {

    InitWindow(m_windowWidth, m_windowHeight, "Game of Life");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    // SetTargetFPS(60);

    Clear();
    m_rules = RuleSet::Parse(m_ruleString);
}

Game::Impl::~Impl() { CloseWindow(); }

void Game::Impl::Update(float dt) {
    // Update screen dimensions
    m_windowWidth = GetScreenWidth();
    m_windowHeight = GetScreenHeight();

    m_camera.Update(dt, m_windowWidth, m_windowHeight);

    // Mouse inputs
    Vector2 mousePos = m_camera.GetMouseWorldPos();
    int mouseX = static_cast<int>(std::floor(mousePos.x / CELL_SIZE));
    int mouseY = static_cast<int>(std::floor(mousePos.y / CELL_SIZE));
    m_mouseTarget = { mouseX, mouseY };

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        ToggleCell(m_mouseTarget);
    }

    // Keyboard inputs
    if (IsKeyPressed(KEY_X)) {
        Simluation::Step(m_chunks, m_nextChunks, m_rules);
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
    } else if (IsKeyPressed(KEY_S)) {
        m_updateInterval -= 0.05f;
        m_updateInterval = Clamp(m_updateInterval, 0.05f, 2.0f);
    }
    if (IsKeyPressed(KEY_F3)) {
        m_showDebugInfo ^= 1;
    }

    // Update timer
    m_timer += GetFrameTime();
    if (m_simulating && m_timer >= m_updateInterval) {
        Simluation::Step(m_chunks, m_nextChunks, m_rules);
        m_timer = 0.0f;
    }
}

void Game::Impl::Draw() {
    BeginDrawing();
    ClearBackground(BLACK);

    BeginMode2D(m_camera.GetCamera());

    Vector2 topLeftWorld = m_camera.GetTopLeftWorld();
    Vector2 bottomRightWorld = m_camera.GetBottomRightWorld(m_windowWidth, m_windowHeight);

    int minChunkX = static_cast<int>(std::floor(topLeftWorld.x / CHUNK_WORLD_SIZE));
    int maxChunkX = static_cast<int>(std::floor(bottomRightWorld.x / CHUNK_WORLD_SIZE));
    int minChunkY = static_cast<int>(std::floor(topLeftWorld.y / CHUNK_WORLD_SIZE));
    int maxChunkY = static_cast<int>(std::floor(bottomRightWorld.y / CHUNK_WORLD_SIZE));

    // Source rectangle is always the full 64x64 texture
    const Rectangle sourceRect = { 0.0f, 0.0f, 64.0f, 64.0f };

    for (auto& [chunkPos, chunk] : m_chunks) {
        if (chunkPos.x < minChunkX || chunkPos.x > maxChunkX || chunkPos.y < minChunkY ||
            chunkPos.y > maxChunkY) {
            continue;
        }

        // Top-left world coordinate of this chunk
        float chunkBaseX = static_cast<float>(chunkPos.x) * CHUNK_WORLD_SIZE;
        float chunkBaseY = static_cast<float>(chunkPos.y) * CHUNK_WORLD_SIZE;

        // Destination rectangle in world space
        Rectangle destRect = { chunkBaseX, chunkBaseY, CHUNK_WORLD_SIZE, CHUNK_WORLD_SIZE };

        // Ensure the GPU texture is up-to-date before drawing
        if (chunk.isDirty) {
            chunk.UpdateChunkTexture();
        }

        // Draw the ENTIRE 64x64 chunk in a single GPU call
        DrawTexturePro(chunk.texture, sourceRect, destRect, { 0.0f, 0.0f }, 0.0f, WHITE);
    }

    EndMode2D();

    if (m_showDebugInfo) {
        Color color = BLUE;

        auto DrawDebugText = [&, row = 10](std::string_view s, auto... args) mutable {
            DrawText(TextFormat(s.data(), (args)...), 10, row, 30, color);
            row += 35;
        };

        DrawDebugText("%2d FPS", GetFPS());
        DrawDebugText("Rule string: %s", m_ruleString.data());
        DrawDebugText("Update interval: %.2f", m_updateInterval);
        DrawDebugText("Simulation speed: %.2f", 1.0f / m_updateInterval);
        DrawDebugText("Mouse target cell: %d, %d", m_mouseTarget.x, m_mouseTarget.y);
        DrawDebugText("Mouse position: %d, %d", GetMouseX(), GetMouseY());
    }

    EndDrawing();
}

void Game::Impl::ToggleCell(const Vec2i& pos) {
    LocalCellPos local = GlobalToLocal(pos);

    Chunk& chunk = m_chunks[local.chunkPos];

    chunk.ToggleCell(local.localPos);

    if (chunk.IsEmpty()) {
        m_chunks.erase(local.chunkPos);
    }
}

void Game::Impl::Clear() {
    m_simulating = false;
    m_chunks.clear();
}

void Game::Impl::Randomize() {
    Clear();

    Vector2 topLeftWorld = m_camera.GetTopLeftWorld();
    Vector2 bottomRightWorld = m_camera.GetBottomRightWorld(m_windowWidth, m_windowHeight);

    int minX = static_cast<int>(std::floor(topLeftWorld.x / CELL_SIZE));
    int maxX = static_cast<int>(std::ceil(bottomRightWorld.x / CELL_SIZE));
    int minY = static_cast<int>(std::floor(topLeftWorld.y / CELL_SIZE));
    int maxY = static_cast<int>(std::ceil(bottomRightWorld.y / CELL_SIZE));

    for (int x = minX; x <= maxX; ++x) {
        for (int y = minY; y <= maxY; ++y) {
            if (GetRandomValue(1, 3) == 1) {
                LocalCellPos local = GlobalToLocal({ x, y });
                m_chunks[local.chunkPos].SetCell(local.localPos);
            }
        }
    }
}

// Public Game API Wrapper
Game::Game(const GameSpecs& specs) : m_impl(std::make_unique<Impl>(specs)) {}
Game::~Game() = default;

Game::Game(Game&&) noexcept = default;
Game& Game::operator=(Game&&) noexcept = default;

void Game::Run() {
    while (!WindowShouldClose()) {
        m_impl->Update(GetFrameTime());
        m_impl->Draw();
    }
}

}
