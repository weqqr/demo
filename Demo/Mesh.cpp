#include <Demo/Mesh.h>

namespace Demo {
VertexLayout Vertex::layout()
{
    return VertexLayout{
        .binding_descriptions = {
            VkVertexInputBindingDescription{
                .binding = 0,
                .stride = 8 * sizeof(float),
                .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
            },
        },
        .attribute_descriptions = {
            VkVertexInputAttributeDescription{
                .location = 0,
                .binding = 0,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .offset = 0,
            },
            VkVertexInputAttributeDescription{
                .location = 1,
                .binding = 0,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .offset = 3 * sizeof(float),
            },
            VkVertexInputAttributeDescription{
                .location = 2,
                .binding = 0,
                .format = VK_FORMAT_R32G32_SFLOAT,
                .offset = (3 + 3) * sizeof(float),
            },
        },
    };
}

void Mesh::add_vertex(Vertex vertex)
{
    m_data.push_back(vertex.position.x);
    m_data.push_back(vertex.position.y);
    m_data.push_back(vertex.position.z);
    m_data.push_back(vertex.normal.x);
    m_data.push_back(vertex.normal.y);
    m_data.push_back(vertex.normal.z);
    m_data.push_back(vertex.uv.x);
    m_data.push_back(vertex.uv.y);

    m_vertex_count++;
}

GPUMesh::GPUMesh(VmaAllocator allocator, const Mesh& mesh)
{
    m_buffer = Buffer(allocator, mesh.data().size() * sizeof(float), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
    m_buffer.map([&](auto* ptr) {
        memcpy(ptr, mesh.data().data(), mesh.data().size() * sizeof(float));
    });
}
}
