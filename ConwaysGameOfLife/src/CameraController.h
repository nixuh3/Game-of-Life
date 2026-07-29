#pragma once

#include <raylib.h>

namespace GoL {

class CameraController {
  public:
    CameraController(int windowWidth, int windowHeight);

    void Update(float dt, int windowWidth, int windowHeight);

    Vector2 ScreenToWorld(Vector2 screenPos) const { return GetScreenToWorld2D(screenPos, m_camera); }
    Vector2 GetMouseWorldPos() const { return GetScreenToWorld2D(GetMousePosition(), m_camera); }
    Vector2 GetTopLeftWorld() const { return ScreenToWorld({ 0.0f, 0.0f }); }
    Vector2 GetBottomRightWorld(int windowWidth, int windowHeight) const {
        return ScreenToWorld({ static_cast<float>(windowWidth), static_cast<float>(windowHeight) });
    }

    Camera2D GetCamera() { return m_camera; }
    const Camera2D& GetCamera() const { return m_camera; }

  private:
    static constexpr float CAMERA_SPEED = 600.0f;
    static constexpr float ZOOM_SPEED = 0.1f;

    Camera2D m_camera;
};

}
