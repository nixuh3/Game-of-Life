#pragma once

#include <SFML/Graphics.hpp>

namespace GoL {

class CameraController {
  public:
    explicit CameraController(sf::Vector2u windowSize);

    void Update(float dt, const sf::RenderWindow& window, float scrollDelta = 0.0f);

    sf::Vector2f ScreenToWorld(sf::Vector2f screenPos, const sf::RenderTarget& target) const {
        return target.mapPixelToCoords(sf::Vector2i(screenPos), m_camera);
    }

    sf::Vector2f GetMouseWorldPos(const sf::RenderWindow& window) const {
        return window.mapPixelToCoords(sf::Mouse::getPosition(window), m_camera);
    }

    sf::Vector2f GetTopLeftWorld(const sf::RenderTarget& target) const { return ScreenToWorld({0.0f, 0.0f}, target); }

    sf::Vector2f GetBottomRightWorld(const sf::RenderTarget& target) const {
        return ScreenToWorld(sf::Vector2f(target.getSize()), target);
    }

    sf::View& GetCamera() { return m_camera; }
    const sf::View& GetCamera() const { return m_camera; }

  private:
    static constexpr float CAMERA_SPEED = 600.0f;
    static constexpr float ZOOM_SPEED   = 0.1f;

    sf::View m_camera;
    sf::Vector2i m_lastMousePos;
    float m_currentZoom = 1.0f;
};

} // namespace GoL
