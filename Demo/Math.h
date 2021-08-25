#pragma once

namespace Demo {
struct Vector2 {
    float x, y;

    Vector2() = default;
    Vector2(float x, float y)
        : x(x)
        , y(y)
    {
    }
};

struct Vector3 {
    float x, y, z;

    Vector3() = default;

    explicit Vector3(float x)
        : x(x)
        , y(x)
        , z(x)
    {
    }

    constexpr Vector3(float x, float y, float z)
        : x(x)
        , y(y)
        , z(z)
    {
    }

    void operator+=(Vector3 rhs);
    void operator-=(Vector3 rhs);
    Vector3 operator*(float rhs) const;
    Vector3 operator/(float rhs) const;
    static constexpr Vector3 up() { return {0.0f, 1.0f, 0.0f}; };
};

float length(Vector3 lhs);
Vector3 normalize(Vector3 lhs);
Vector3 cross(Vector3 lhs, Vector3 rhs);

struct Vector4 {
    float x, y, z, w;

    Vector4() = default;
    Vector4(float x, float y, float z, float w)
        : x(x)
        , y(y)
        , z(z)
        , w(w)
    {
    }

    Vector4(Vector3 xyz, float w)
        : x(xyz.x)
        , y(xyz.y)
        , z(xyz.z)
        , w(w)
    {
    }
};
}
