#include <Demo/FlyCamera.h>

#include <cmath>

namespace Demo {
FlyCamera::FlyCamera(Vector3 position, float mouse_sensitivity, float movement_speed)
{
    m_position = position;
    m_pitch = 0.0f;
    m_yaw = 0.0f;
    m_mouse_sensitivity = mouse_sensitivity;
    m_movement_speed = movement_speed;
}

Vector3 FlyCamera::position() const
{
    return m_position;
}

Vector3 FlyCamera::look_dir() const
{
    float x = std::cos(m_yaw) * std::cos(m_pitch);
    float y = std::sin(m_pitch);
    float z = std::sin(m_yaw) * std::cos(m_pitch);

    return {x, y, z};
}

void FlyCamera::rotate(float dx, float dy)
{
    m_pitch += dy * m_mouse_sensitivity;
    m_yaw -= dx * m_mouse_sensitivity;

    if (m_pitch > 85.5) {
        m_pitch = 85.5;
    } else if (m_pitch < -85.5) {
        m_pitch = -85.5;
    }
}

void FlyCamera::move(MovementDirection direction)
{
    auto dir = look_dir();
    auto right = normalize(cross(dir, Vector3::up()));
    auto forward = normalize(Vector3(dir.x, 0, dir.z));

    switch (direction) {
    case MovementDirection::Forward:
        m_position += forward * m_movement_speed;
        break;
    case MovementDirection::Backward:
        m_position -= forward * m_movement_speed;
        break;
    case MovementDirection::Left:
        m_position -= right * m_movement_speed;
        break;
    case MovementDirection::Right:
        m_position += right * m_movement_speed;
        break;
    case MovementDirection::Up:
        m_position += Vector3::up() * m_movement_speed;
        break;
    case MovementDirection::Down:
        m_position -= Vector3::up() * m_movement_speed;
        break;
    }
}
}
