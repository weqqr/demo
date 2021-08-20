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
    Vector3(float x, float y, float z)
        : x(x)
        , y(y)
        , z(z)
    {
    }
};

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
