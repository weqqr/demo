#include <Demo/Math.h>

#include <cmath>

namespace Demo {
void Vector3::operator+=(Vector3 rhs)
{
    x += rhs.x;
    y += rhs.y;
    z += rhs.z;
}

void Vector3::operator-=(Vector3 rhs)
{
    x -= rhs.x;
    y -= rhs.y;
    z -= rhs.z;
}

Vector3 Vector3::operator*(float rhs) const
{
    return {x * rhs, y * rhs, z * rhs};
}

Vector3 Vector3::operator/(float rhs) const
{
    float reciprocal = 1.0f / rhs;
    return {x * reciprocal, y * reciprocal, z * reciprocal};
}

float length(Vector3 lhs)
{
    return std::sqrt(lhs.x * lhs.x + lhs.y * lhs.y + lhs.z * lhs.z);
}

Vector3 normalize(Vector3 lhs)
{
    return lhs / length(lhs);
}

Vector3 cross(Vector3 lhs, Vector3 rhs)
{
    float x = lhs.y * rhs.z - lhs.z * rhs.y;
    float y = lhs.z * rhs.x - lhs.x * rhs.z;
    float z = lhs.x * rhs.y - lhs.y * rhs.x;

    return {x, y, z};
}
}
