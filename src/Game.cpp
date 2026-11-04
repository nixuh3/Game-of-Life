#include "game.hpp"
#include "math.hpp"
#include "simulation.hpp"
#include <raygui.h>
#include <raymath.h>

namespace gol
{

enum CellType : uint8_t { ALIVE,
    DEAD };

constexpr CellType operator~(CellType type) { return type == ALIVE ? DEAD : ALIVE; }

Game::Game(const GameSpecs& specs)
    : m_windowWidth(specs.windowWidth),
      m_windowHeight(specs.windowHeight),
      m_ruleString(specs.ruleString),
      m_camera(m_windowWidth, m_windowHeight)
{
    InitWindow(m_windowWidth, m_windowHeight, "Game of Life");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(120);

    GuiSetStyle(DEFAULT, TEXT_SIZE, 22);

    m_loader.LoadAllFiles("blueprints");

    Clear();
    m_rules = RuleSet::Parse(m_ruleString);
}

Game::~Game() { CloseWindow(); }

void Game::Run()
{
    while (!WindowShouldClose()) {
        Update(GetFrameTime());
        Draw();
    }
}

void Game::Update(float dt)
{
    // Update screen dimensions
    m_windowWidth  = GetScreenWidth();
    m_windowHeight = GetScreenHeight();

    m_camera.Update(dt, m_windowWidth, m_windowHeight);

    // Mouse inputs
    Vector2 mousePos = m_camera.GetMouseWorldPos();
    int mouseX       = static_cast<int>(std::floor(mousePos.x / CELL_SIZE));
    int mouseY       = static_cast<int>(std::floor(mousePos.y / CELL_SIZE));
    m_mouseTarget    = {mouseX, mouseY};

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !m_showBlueprintGUI) {
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
        if (m_simulating) {
            SetWindowTitle("Game of Life");
            m_simulating = false;
        } else {
            SetWindowTitle("Game of Life (Simulating...)");
            m_simulating = true;
        }
    }
    if (IsKeyPressed(KEY_C)) {
        Clear();
    }

    // Blueprints
    if (IsKeyPressed(KEY_B)) {
        m_showBlueprintGUI ^= 1;
    }
    if (IsKeyPressed(KEY_ENTER)) {
        std::string activeName = m_loader.GetSelectedBlueprintName(); // Add this helper getter if needed
        auto bpOpt             = m_loader.GetBlueprint(activeName);
        if (bpOpt.has_value()) {
            const Blueprint& bp = bpOpt->get();
            for (int row = 0; row < bp.height; ++row) {
                for (int col = 0; col < bp.width; ++col) {
                    if (bp.cells[row][col]) {
                        // Correct offset mapping: World X = mouse.x + col, World Y = mouse.y + row
                        Vec2i worldPos     = {m_mouseTarget.x + col, m_mouseTarget.y + row};
                        LocalCellPos local = GlobalToLocal(worldPos);
                        m_chunks[local.chunkPos].SetCell(local.localPos);
                    }
                }
            }
        }
    }

    auto ClampInterval = [this]() {
        m_updateInterval = Clamp(m_updateInterval, 0.001f, 2.0f);
    };
    if (IsKeyPressed(KEY_L)) {
        m_updateInterval += 0.01f;
        ClampInterval();
    } else if (IsKeyPressed(KEY_H)) {
        m_updateInterval -= 0.01f;
        ClampInterval();
    } else if (IsKeyPressed(KEY_J)) {
        m_updateInterval *= 2;
        ClampInterval();
    } else if (IsKeyPressed(KEY_K)) {
        m_updateInterval /= 2;
        ClampInterval();
    }

    if (IsKeyPressed(KEY_F3)) {
        m_showDebugInfo ^= 1;
    }
    if (IsKeyPressed(KEY_F11)) {
        ToggleFullscreen();
    }

    // Update timer
    m_timer += GetFrameTime();
    if (m_simulating && m_timer >= m_updateInterval) {
        Simluation::Step(m_chunks, m_nextChunks, m_rules);
        m_timer = 0.0f;
    }
}

void Game::Draw()
{
    BeginDrawing();
    ClearBackground(BLACK);

    BeginMode2D(m_camera.GetCamera());

    Vector2 topLeftWorld     = m_camera.GetTopLeftWorld();
    Vector2 bottomRightWorld = m_camera.GetBottomRightWorld(m_windowWidth, m_windowHeight);

    int minChunkX = static_cast<int>(std::floor(topLeftWorld.x / CHUNK_WORLD_SIZE));
    int maxChunkX = static_cast<int>(std::floor(bottomRightWorld.x / CHUNK_WORLD_SIZE));
    int minChunkY = static_cast<int>(std::floor(topLeftWorld.y / CHUNK_WORLD_SIZE));
    int maxChunkY = static_cast<int>(std::floor(bottomRightWorld.y / CHUNK_WORLD_SIZE));

    // Source rectangle is always the full 64x64 texture
    const Rectangle sourceRect = {0.0f, 0.0f, 64.0f, 64.0f};

    for (auto& [chunkPos, chunk] : m_chunks) {
        if (chunkPos.x < minChunkX || chunkPos.x > maxChunkX || chunkPos.y < minChunkY ||
            chunkPos.y > maxChunkY) {
            continue;
        }

        // Top-left world coordinate of this chunk
        float chunkBaseX = static_cast<float>(chunkPos.x) * CHUNK_WORLD_SIZE;
        float chunkBaseY = static_cast<float>(chunkPos.y) * CHUNK_WORLD_SIZE;

        // Destination rectangle in world space
        Rectangle destRect = {chunkBaseX, chunkBaseY, CHUNK_WORLD_SIZE, CHUNK_WORLD_SIZE};

        // Ensure the GPU texture is up-to-date before drawing
        if (chunk.isDirty) {
            chunk.UpdateChunkTexture();
        }

        // Draw the ENTIRE 64x64 chunk in a single GPU call
        DrawTexturePro(chunk.texture, sourceRect, destRect, {0.0f, 0.0f}, 0.0f, WHITE);
    }

    EndMode2D();

    if (m_showBlueprintGUI)
        m_loader.Render();

    if (m_showDebugInfo) {
        Color color = BLUE;

        auto DrawDebugText = [&, row = 10](std::string_view s, auto... args) mutable {
            DrawText(TextFormat(s.data(), (args)...), 10, row, 30, color);
            row += 35;
        };

        DrawDebugText("%2d FPS", GetFPS());
        DrawDebugText("Simulating: %s", m_simulating ? "true" : "false");
        DrawDebugText("Rule string: %s", m_ruleString.data());
        DrawDebugText("Update interval: %.2f (%.2f/s)", m_updateInterval, 1.0f / m_updateInterval);
        DrawDebugText("Mouse target cell: %d, %d", m_mouseTarget.x, m_mouseTarget.y);
        DrawDebugText("Mouse position: %d, %d", GetMouseX(), GetMouseY());
    }

    EndDrawing();
}

void Game::ToggleCell(const Vec2i& pos)
{
    LocalCellPos local = GlobalToLocal(pos);

    Chunk& chunk = m_chunks[local.chunkPos];

    chunk.ToggleCell(local.localPos);

    if (chunk.IsEmpty()) {
        m_chunks.erase(local.chunkPos);
    }
}

void Game::Clear()
{
    m_simulating = false;
    m_chunks.clear();
    SetWindowTitle("Game of Life");
}

void Game::Randomize()
{
    Clear();

    Vector2 topLeftWorld     = m_camera.GetTopLeftWorld();
    Vector2 bottomRightWorld = m_camera.GetBottomRightWorld(m_windowWidth, m_windowHeight);

    int minX = static_cast<int>(std::floor(topLeftWorld.x / CELL_SIZE));
    int maxX = static_cast<int>(std::ceil(bottomRightWorld.x / CELL_SIZE));
    int minY = static_cast<int>(std::floor(topLeftWorld.y / CELL_SIZE));
    int maxY = static_cast<int>(std::ceil(bottomRightWorld.y / CELL_SIZE));

    for (int x = minX; x <= maxX; ++x) {
        for (int y = minY; y <= maxY; ++y) {
            if (GetRandomValue(1, 3) == 1) {
                LocalCellPos local = GlobalToLocal({x, y});
                m_chunks[local.chunkPos].SetCell(local.localPos);
            }
        }
    }
}

} // namespace gol
