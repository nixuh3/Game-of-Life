#include "camera.hpp"
#include <raymath.h>
#include <cmath>

namespace gol {

CameraController::CameraController(int windowWidth, int windowHeight) {
    m_camera.offset = { static_cast<float>(windowWidth) / 2.0f, static_cast<float>(windowHeight) / 2.0f };
    m_camera.target = { 0.0f, 0.0f };
    m_camera.rotation = 0.0f;
    m_camera.zoom = 1.0f;
}

void CameraController::Update(float dt, int windowWidth, int windowHeight) {
    m_camera.offset = { static_cast<float>(windowWidth) / 2.0f, static_cast<float>(windowHeight) / 2.0f };

    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
        Vector2 delta = GetMouseDelta();

        m_camera.target.x -= delta.x / m_camera.zoom;
        m_camera.target.y -= delta.y / m_camera.zoom;
    }
    float wheel = GetMouseWheelMove();
    if (wheel != 0.0f) {
        Vector2 mouseWorldBefore = GetScreenToWorld2D(GetMousePosition(), m_camera);

        m_camera.zoom = std::exp(std::log(m_camera.zoom) + wheel * ZOOM_SPEED);
        m_camera.zoom = Clamp(m_camera.zoom, 0.05f, 3.0f);

        Vector2 mouseWorldAfter = GetScreenToWorld2D(GetMousePosition(), m_camera);

        m_camera.target.x += (mouseWorldBefore.x - mouseWorldAfter.x);
        m_camera.target.y += (mouseWorldBefore.y - mouseWorldAfter.y);
    }

    m_camera.target.x += static_cast<float>(IsKeyDown(KEY_RIGHT) - IsKeyDown(KEY_LEFT)) * dt * CAMERA_SPEED;
    m_camera.target.y += static_cast<float>(IsKeyDown(KEY_DOWN) - IsKeyDown(KEY_UP)) * dt * CAMERA_SPEED;
}

}
