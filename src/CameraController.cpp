#include "CameraController.h"
#include <cmath>

namespace GoL {

CameraController::CameraController(sf::Vector2u windowSize) {
    m_camera.setCenter(sf::Vector2f(windowSize) / 2.0f);
    m_camera.setSize(sf::Vector2f(windowSize));
}

void CameraController::Update(float dt, const sf::RenderWindow& window, float scrollDelta) {
    // Panning
    sf::Vector2i currentMousePos = sf::Mouse::getPosition(window);
    if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Right)) {
        // Convert mouse positions to world space to get the actual movement delta
        sf::Vector2f oldPos = window.mapPixelToCoords(m_lastMousePos, m_camera);
        sf::Vector2f newPos = window.mapPixelToCoords(currentMousePos, m_camera);

        // Move camera in the opposite direction of the mouse swipe
        m_camera.move(oldPos - newPos);
    }
    m_lastMousePos = currentMousePos;

    // Zooming
    if (scrollDelta != 0.0f) {
        // Grab the mouse position in world space BEFORE zooming
        sf::Vector2f mouseWorldBefore = window.mapPixelToCoords(currentMousePos, m_camera);

        // Calculate the new zoom factor and clamp it
        // Note: SFML zooms OUT with larger values, so we flip the sign from Raylib
        float desiredZoom = std::exp(std::log(m_currentZoom) - scrollDelta * ZOOM_SPEED);
        float newZoom     = std::clamp(desiredZoom, 0.2f, 5.0f);

        // Calculate the relative factor to apply to the SFML view
        float factor = newZoom / m_currentZoom;
        m_camera.zoom(factor);
        m_currentZoom = newZoom;

        // Grab the mouse position in world space AFTER zooming
        sf::Vector2f mouseWorldAfter = window.mapPixelToCoords(currentMousePos, m_camera);

        // Shift the camera so the mouse stays over the same world coordinate
        m_camera.move(mouseWorldBefore - mouseWorldAfter);
    }

    // Movement
    sf::Vector2i d = {
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) - sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left),
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down) - sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)};

    m_camera.move(static_cast<sf::Vector2f>(d) * dt * CAMERA_SPEED * m_currentZoom);
}

} // namespace GoL
