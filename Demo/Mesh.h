#pragma once

#include <Demo/Buffer.h>
#include <Demo/RendererBase.h>
#include <Demo/Math.h>
#include <vk_mem_alloc.h>

#include <vector>

namespace Demo {
struct VertexLayout {
    std::vector<VkVertexInputBindingDescription> binding_descriptions;
    std::vector<VkVertexInputAttributeDescription> attribute_descriptions;
};

struct Vertex {
    Vector3 position;
    Vector3 normal;
    Vector2 uv;

    static VertexLayout layout();
};

class Mesh {
public:
    Mesh() = default;
    void add_vertex(Vertex vertex);
    const std::vector<float>& data() const { return m_data; }
    uint32_t vertex_count() const { return m_vertex_count; }

private:
    uint32_t m_vertex_count = 0;
    std::vector<float> m_data;
};

class GPUMesh {
public:
    GPUMesh() = default;
    GPUMesh(VmaAllocator allocator, const Mesh& mesh);
    uint32_t vertex_count() const { return m_vertex_count; }
    const Buffer& buffer() const { return m_buffer; }

private:
    uint32_t m_vertex_count = 0;
    Buffer m_buffer = {};
};
}
