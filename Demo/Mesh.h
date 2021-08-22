#pragma once

#include <Demo/Buffer.h>
#include <Demo/RendererBase.h>
#include <Demo/Math.h>

#include <vector>

namespace Demo {
struct Vertex {
    Vector3 position;
    Vector3 normal;
    Vector2 uv;

    static void layout();
};

class GPUMesh {
public:
    GPUMesh() = default;

private:
    Buffer m_buffer = {};
};

class Mesh {
public:
    Mesh() = default;
    void add_vertex(Vertex vertex);
    const std::vector<float>& data() const { return m_data; }

private:
    std::vector<float> m_data;
};
}
