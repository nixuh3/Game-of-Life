#include "Game.h"

#include <raylib.h>
#include <raymath.h>
#include <cmath>
#include <cctype>
#include <bit>
#include <unordered_set>
#include <cstdint>
#include <string>

#include "Chunk.h"
#include "Math.h"

namespace GoL {

enum CellType : uint8_t { ALIVE, DEAD };

constexpr CellType operator~(CellType type) { return type == ALIVE ? DEAD : ALIVE; }

struct Game::Impl {
    static constexpr float CELL_SIZE = 20.0f;
    static constexpr float CHUNK_WORLD_SIZE = 64.0f * CELL_SIZE;
    static constexpr float CAMERA_SPEED = 600.0f;
    static constexpr float ZOOM_SPEED = 60.0f;

    int m_windowWidth;
    int m_windowHeight;

    Camera2D m_camera{};

    ChunkMap m_chunks;
    ChunkMap m_nextChunks;

    std::string m_ruleString;
    uint16_t m_birthMask = 0;
    uint16_t m_survivalMask = 0;

    std::unordered_map<Vec2i, int, Vec2i::Hash> m_neighborCounts;

    Vec2i m_mouseTarget = {};
    bool m_simulating = false;
    bool m_showDebugInfo = false;
    float m_timer = 0.0f;
    float m_updateInterval = 0.1f;

    explicit Impl(const GameSpecs& specs);
    ~Impl();

    void Update(float dt);
    void Draw() const;
    void ToggleCell(const Vec2i& pos);
    void Clear();
    void Randomize();
    void Step();
    void ParseRuleString();
};

Game::Impl::Impl(const GameSpecs& specs)
    : m_windowWidth(specs.windowWidth), m_windowHeight(specs.windowHeight), m_ruleString(specs.ruleString) {
    InitWindow(m_windowWidth, m_windowHeight, "Game of Life");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    // SetTargetFPS(60);

    Clear();
    ParseRuleString();

    m_camera.offset = { static_cast<float>(m_windowWidth) / 2.0f, static_cast<float>(m_windowHeight) / 2.0f };
    m_camera.target = { 0.0f, 0.0f };
    m_camera.rotation = 0.0f;
    m_camera.zoom = 1.0f;
}

Game::Impl::~Impl() { CloseWindow(); }

void Game::Impl::Update(float dt) {
    // Update screen dimensions
    m_windowWidth = GetScreenWidth();
    m_windowHeight = GetScreenHeight();
    m_camera.offset = { static_cast<float>(m_windowWidth) / 2.0f, static_cast<float>(m_windowHeight) / 2.0f };

    // Mouse inputs
    Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), m_camera);
    int mouseX = static_cast<int>(std::floor(mousePos.x / CELL_SIZE));
    int mouseY = static_cast<int>(std::floor(mousePos.y / CELL_SIZE));
    m_mouseTarget = { mouseX, mouseY };

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        ToggleCell(m_mouseTarget);
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
        m_camera.zoom = Clamp(m_camera.zoom, 0.05f, 3.0f);

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
        Step();
        m_timer = 0.0f;
    }
}

void Game::Impl::Draw() const {
    BeginDrawing();
    ClearBackground(BLACK);

    BeginMode2D(m_camera);

    Vector2 topLeftWorld = GetScreenToWorld2D({ 0.0f, 0.0f }, m_camera);
    Vector2 bottomRightWorld = GetScreenToWorld2D(
        { static_cast<float>(m_windowWidth), static_cast<float>(m_windowHeight) }, m_camera);

    int minChunkX = static_cast<int>(std::floor(topLeftWorld.x / CHUNK_WORLD_SIZE));
    int maxChunkX = static_cast<int>(std::floor(bottomRightWorld.x / CHUNK_WORLD_SIZE));
    int minChunkY = static_cast<int>(std::floor(topLeftWorld.y / CHUNK_WORLD_SIZE));
    int maxChunkY = static_cast<int>(std::floor(bottomRightWorld.y / CHUNK_WORLD_SIZE));

    for (const auto& [chunkPos, chunk] : m_chunks) {
        if (chunkPos.x < minChunkX || chunkPos.x > maxChunkX || chunkPos.y < minChunkY ||
            chunkPos.y > maxChunkY) {
            continue;
        }

        // Base world coordinates of the top-left corner of this chunk
        float chunkBaseX = static_cast<float>(chunkPos.x * 64) * CELL_SIZE;
        float chunkBaseY = static_cast<float>(chunkPos.y * 64) * CELL_SIZE;

        for (int r = 0; r < 64; ++r) {
            uint64_t rowBits = chunk.rows[r];
            float cellY = chunkBaseY + static_cast<float>(r) * CELL_SIZE;

            // Loop only as many times as there are live cells in this row
            while (rowBits != 0) {
                // Find the index of the lowest set bit
                int c = std::countr_zero(rowBits);

                float cellX = chunkBaseX + static_cast<float>(c) * CELL_SIZE;
                DrawRectangleRec({ cellX, cellY, CELL_SIZE, CELL_SIZE }, GREEN);

                // Clear the lowest set bit to move to the next live cell
                rowBits &= (rowBits - 1);
            }
        }
    }

    EndMode2D();

    if (m_showDebugInfo) {
        Color color = BLUE;

        auto DrawDebugText = [&, row = 10](std::string_view s, auto... args) mutable {
            DrawText(TextFormat(s.data(), (args)...), 10, row, 30, color);
            row += 35;
        };

        DrawDebugText("%2d FPS", GetFPS());
        DrawDebugText("Rule string: %s", m_ruleString.c_str());
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
                LocalCellPos local = GlobalToLocal({ x, y });
                m_chunks[local.chunkPos].SetCell(local.localPos);
            }
        }
    }
}

void Game::Impl::Step() {
    m_nextChunks.clear();

    // Any active chunk + all 8 of its surrounding empty chunks
    std::unordered_set<Vec2i, Vec2i::Hash> chunksToProcess;
    for (const auto& [pos, chunk] : m_chunks) {
        if (chunk.IsEmpty()) {
            continue;
        }

        for (int dy = -1; dy <= 1; ++dy) {
            for (int dx = -1; dx <= 1; ++dx) {
                chunksToProcess.insert(pos + Vec2i{ dx, dy });
            }
        }
    }

    // Evaluate each chunk in parallel across its 64-bit rows
    for (const Vec2i& chunkPos : chunksToProcess) {
        // Pre-fetch pointers to the 3x3 neighborhood of chunks to make border lookups instant
        const Chunk* neighbors[3][3] = { nullptr };
        for (int dy = -1; dy <= 1; ++dy) {
            for (int dx = -1; dx <= 1; ++dx) {
                auto it = m_chunks.find(chunkPos + Vec2i{ dx, dy });
                if (it != m_chunks.end()) {
                    neighbors[dy + 1][dx + 1] = &it->second;
                }
            }
        }

        // Safely retrieves a row word even across chunk vertical/horizontal boundaries
        auto GetChunkRow = [&](int dx, int r) -> uint64_t {
            int dy = 0;
            if (r < 0) {
                dy = -1;
                r = 63;
            } else if (r > 63) {
                dy = 1;
                r = 0;
            }

            const Chunk* c = neighbors[dy + 1][dx + 1];
            return c ? c->rows[r] : 0ULL;
        };

        Chunk nextChunk;
        bool hasLiveCells = false;

        for (int r = 0; r < 64; ++r) {
            // Fetch the 3 raw horizontal rows (North, Center, South) and their West/East neighbors
            uint64_t nRow = GetChunkRow(0, r - 1);
            uint64_t nWest = GetChunkRow(-1, r - 1);
            uint64_t nEast = GetChunkRow(1, r - 1);

            uint64_t cRow = GetChunkRow(0, r);
            uint64_t cWest = GetChunkRow(-1, r);
            uint64_t cEast = GetChunkRow(1, r);

            uint64_t sRow = GetChunkRow(0, r + 1);
            uint64_t sWest = GetChunkRow(-1, r + 1);
            uint64_t sEast = GetChunkRow(1, r + 1);

            // Construct the 8 directional neighbor bitboards by shifting across borders
            // (Assuming bit 0 is x=0, shifting left by 1 moves West neighbors into position)
            uint64_t nw = (nRow << 1) | (nWest >> 63);
            uint64_t n = nRow;
            uint64_t ne = (nRow >> 1) | (nEast << 63);

            uint64_t w = (cRow << 1) | (cWest >> 63);
            uint64_t e = (cRow >> 1) | (cEast << 63);

            uint64_t sw = (sRow << 1) | (sWest >> 63);
            uint64_t s = sRow;
            uint64_t se = (sRow >> 1) | (sEast << 63);

            // PARALLEL BITWISE ADDITION:
            // Sum the 8 neighbor boards into a 4-bit binary counter per cell (c3, c2, c1, c0)
            uint64_t w1a, w2a, w1b, w2b, w1c, w2c, w2d;
            uint64_t w2Sum, w4a, w4b;
            uint64_t c0, c1, c2, c3;

            FullAdd(nw, n, ne, w1a, w2a);
            FullAdd(w, e, sw, w1b, w2b);
            FullAdd(w1a, w1b, s, w1c, w2c);
            HalfAdd(w1c, se, c0, w2d); // c0 is the 1s bit of the count

            FullAdd(w2a, w2b, w2c, w2Sum, w4a);
            HalfAdd(w2Sum, w2d, c1, w4b); // c1 is the 2s bit of the count

            HalfAdd(w4a, w4b, c2, c3); // c2 is the 4s bit, c3 is the 8s bit

            // Decode the 4-bit counter into exact match masks for counts 0 through 8
            uint64_t eq[9];
            eq[0] = ~c3 & ~c2 & ~c1 & ~c0;
            eq[1] = ~c3 & ~c2 & ~c1 & c0;
            eq[2] = ~c3 & ~c2 & c1 & ~c0;
            eq[3] = ~c3 & ~c2 & c1 & c0;
            eq[4] = ~c3 & c2 & ~c1 & ~c0;
            eq[5] = ~c3 & c2 & ~c1 & c0;
            eq[6] = ~c3 & c2 & c1 & ~c0;
            eq[7] = ~c3 & c2 & c1 & c0;
            eq[8] = c3 & ~c2 & ~c1 & ~c0;

            uint64_t birth_mask = 0;
            uint64_t survival_mask = 0;
            for (int i = 0; i <= 8; ++i) {
                if (m_birthMask & (1 << i)) {
                    birth_mask |= eq[i];
                }
                if (m_survivalMask & (1 << i)) {
                    survival_mask |= eq[i];
                }
            }

            // A dead cell spawns if in birth_mask; a live cell survives if in survival_mask
            uint64_t nextRow = (~cRow & birth_mask) | (cRow & survival_mask);

            nextChunk.rows[r] = nextRow;
            if (nextRow != 0) {
                hasLiveCells = true;
            }
        }

        // Only save the chunk if it contains at least one living cell
        if (hasLiveCells) {
            m_nextChunks[chunkPos] = nextChunk;
        }
    }

    std::swap(m_chunks, m_nextChunks);
}

void Game::Impl::ParseRuleString() {
    m_birthMask = 0;
    m_survivalMask = 0;

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
                m_birthMask |= (1 << digit);
            } else if (mode == ParseMode::SURVIVAL) {
                m_survivalMask |= (1 << digit);
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
