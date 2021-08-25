#pragma once

#include <Demo/Math.h>

namespace Demo {
enum class MovementDirection {
    Forward,
    Backward,
    Left,
    Right,
    Up,
    Down,
};

class FlyCamera {
public:
    FlyCamera(Vector3 position, float mouse_sensitivity, float movement_speed);

    Vector3 position() const;
    Vector3 look_dir() const;

    void rotate(float dx, float dy);
    void move(MovementDirection direction);

private:
    Vector3 m_position = {};
    float m_pitch;
    float m_yaw;
    float m_mouse_sensitivity;
    float m_movement_speed;
};
}
