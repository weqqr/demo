#pragma once

#include <Demo/Math.h>

#include <vector>

namespace Demo {
struct Vertex {
    Vector3 position;
    Vector3 normal;
    Vector2 uv;
};

class Mesh {
public:
    Mesh() = default;
    void add_vertex();

private:
    std::vector<float> m_data;
};
}
