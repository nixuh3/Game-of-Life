#include "Game.h"
#include "Simulation.h"

namespace GoL {

Game::Game(const GameSpecs& specs)
    : m_windowSize(specs.windowSize),
      m_ruleString(specs.ruleString),
      m_camera(m_windowSize),
      m_font("assets/arial.ttf"),
      m_gen(std::random_device{}()) {

    m_window.create(sf::VideoMode(m_windowSize), "Game of Life");
    Clear();
    m_rules = RuleSet::Parse(m_ruleString);
}

void Game::Run() {
    while (m_window.isOpen()) {
        float dt = m_clock.restart().asSeconds();
        Update(dt);
        Draw(dt);
    }
}

void Game::Update(float dt) {
    float scrollDelta = 0.0f;

    while (const std::optional event = m_window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            m_window.close();
        } else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
            switch (keyPressed->scancode) {
            case sf::Keyboard::Scancode::Escape: m_window.close(); break;
            case sf::Keyboard::Scancode::X:      Simluation::Step(m_chunks, m_nextChunks, m_rules); break;
            case sf::Keyboard::Scancode::R:      Randomize(); break;
            case sf::Keyboard::Scancode::Space:  m_simulating ^= 1; break;
            case sf::Keyboard::Scancode::C:      Clear(); break;
            case sf::Keyboard::Scancode::W:
                m_updateInterval += 0.05f;
                m_updateInterval = std::clamp(m_updateInterval, 0.05f, 2.0f);
                break;
            case sf::Keyboard::Scancode::S:
                m_updateInterval -= 0.05f;
                m_updateInterval = std::clamp(m_updateInterval, 0.05f, 2.0f);
                break;
            case sf::Keyboard::Scancode::F3: m_showDebugInfo ^= 1; break;
            default:                         break;
            }

        } else if (const auto* resized = event->getIf<sf::Event::Resized>()) {
            m_windowSize = resized->size;
        } else if (const auto* mouseButtonPressed = event->getIf<sf::Event::MouseButtonPressed>()) {
            if (mouseButtonPressed->button == sf::Mouse::Button::Left) {
                ToggleCell(m_mouseTarget);
            }
        } else if (const auto* wheel = event->getIf<sf::Event::MouseWheelScrolled>()) {
            scrollDelta = wheel->delta;
        }
    }

    m_camera.Update(dt, m_window, scrollDelta);

    sf::Vector2 mousePos = m_camera.GetMouseWorldPos(m_window);
    int mouseX           = static_cast<int>(std::floor(mousePos.x / CELL_SIZE));
    int mouseY           = static_cast<int>(std::floor(mousePos.y / CELL_SIZE));
    m_mouseTarget        = {mouseX, mouseY};

    // Update timer
    m_timer += dt;
    if (m_simulating && m_timer >= m_updateInterval) {
        Simluation::Step(m_chunks, m_nextChunks, m_rules);
        m_timer = 0.0f;
    }
}

void Game::Draw(float dt) {
    m_window.clear(sf::Color::Black);
    m_window.setView(m_camera.GetCamera());

    sf::Vector2f topLeftWorld     = m_camera.GetTopLeftWorld(m_window);
    sf::Vector2f bottomRightWorld = m_camera.GetBottomRightWorld(m_window);

    int minChunkX = static_cast<int>(std::floor(topLeftWorld.x / CHUNK_WORLD_SIZE));
    int maxChunkX = static_cast<int>(std::floor(bottomRightWorld.x / CHUNK_WORLD_SIZE));
    int minChunkY = static_cast<int>(std::floor(topLeftWorld.y / CHUNK_WORLD_SIZE));
    int maxChunkY = static_cast<int>(std::floor(bottomRightWorld.y / CHUNK_WORLD_SIZE));

    for (auto& [chunkPos, chunk] : m_chunks) {
        if (chunkPos.x < minChunkX || chunkPos.x > maxChunkX || chunkPos.y < minChunkY || chunkPos.y > maxChunkY) {
            continue;
        }

        chunk.UpdateChunkTexture();

        // Top-left world coordinate of this chunk
        float chunkBaseX = static_cast<float>(chunkPos.x) * CHUNK_WORLD_SIZE;
        float chunkBaseY = static_cast<float>(chunkPos.y) * CHUNK_WORLD_SIZE;

        // Destination rectangle in world space
        // 2. Configure the sprite for this specific chunk
        sf::Sprite chunkSprite(chunk.texture);
        chunkSprite.setPosition({chunkBaseX, chunkBaseY});

        // Scale the 64x64 texture to fit the world size.
        // Since CHUNK_WORLD_SIZE is 64 * CELL_SIZE, the scale factor is exactly CELL_SIZE.
        chunkSprite.setScale({CELL_SIZE, CELL_SIZE});

        m_window.draw(chunkSprite);
    }

    m_window.setView(m_window.getDefaultView());

    if (m_showDebugInfo) {
        sf::Text text(m_font);
        text.setCharacterSize(30);
        text.setFillColor(sf::Color::Red);

        auto DrawDebugText = [&, row = 10.0f](const std::string& s) mutable {
            text.setString(s);
            text.setPosition({10.0f, row});
            m_window.draw(text);
            row += 35.0f;
        };

        DrawDebugText("FPS: " + std::to_string(static_cast<int>(1.0f / dt)));
        DrawDebugText("Rule string: " + std::string(m_ruleString));
        DrawDebugText("Update interval: " + std::to_string(m_updateInterval));
        DrawDebugText("Simulation speed: " + std::to_string(1.0f / m_updateInterval));
        DrawDebugText("Mouse target cell: " + std::to_string(m_mouseTarget.x) + ", " + std::to_string(m_mouseTarget.y));
        sf::Vector2i mouseScreenPos = sf::Mouse::getPosition(m_window);
        DrawDebugText("Mouse position: " + std::to_string(mouseScreenPos.x) + ", " + std::to_string(mouseScreenPos.y));
    }

    m_window.display();
}

void Game::ToggleCell(sf::Vector2i pos) {
    LocalCellPos local = GlobalToLocal(pos);

    Chunk& chunk = m_chunks[local.chunkPos];

    chunk.ToggleCell(local.localPos);

    if (chunk.IsEmpty()) {
        m_chunks.erase(local.chunkPos);
    }
}

void Game::Clear() {
    m_simulating = false;
    m_chunks.clear();
}

void Game::Randomize() {
    Clear();

    sf::Vector2f topLeftWorld     = m_camera.GetTopLeftWorld(m_window);
    sf::Vector2f bottomRightWorld = m_camera.GetBottomRightWorld(m_window);

    int minX = static_cast<int>(std::floor(topLeftWorld.x / CELL_SIZE));
    int maxX = static_cast<int>(std::ceil(bottomRightWorld.x / CELL_SIZE));
    int minY = static_cast<int>(std::floor(topLeftWorld.y / CELL_SIZE));
    int maxY = static_cast<int>(std::ceil(bottomRightWorld.y / CELL_SIZE));

    for (int x = minX; x <= maxX; ++x) {
        for (int y = minY; y <= maxY; ++y) {
            if (std::uniform_int_distribution(1, 3)(m_gen) == 1) {
                LocalCellPos local = GlobalToLocal({x, y});
                m_chunks[local.chunkPos].SetCell(local.localPos);
            }
        }
    }
}

} // namespace GoL
