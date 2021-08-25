#pragma once

#include <Demo/Mesh.h>
#include <Demo/RenderPass.h>
#include <Demo/RendererBase.h>
#include <Demo/Shader.h>
#include <vector>
#include <optional>

namespace Demo {
struct GraphicsPipelineDesc {
    VkDevice device;
    std::optional<VertexLayout> vertex_layout;
    std::vector<VkDescriptorSetLayout> descriptor_set_layouts;
    std::vector<VkPushConstantRange> push_constant_ranges;

    Shader vertex_shader;
    Shader fragment_shader;
    std::vector<VkFormat> images;
};

class GraphicsPipeline : NonCopyable {
public:
    GraphicsPipeline() = default;
    GraphicsPipeline(GraphicsPipelineDesc desc);
    ~GraphicsPipeline();

    VkPipelineLayout layout() const { return m_layout; }
    VkPipeline raw() const { return m_pipeline; }

    GraphicsPipeline(GraphicsPipeline&& other) noexcept
    {
        *this = move(other);
    }

    GraphicsPipeline& operator=(GraphicsPipeline&& other) noexcept
    {
        swap(m_device, other.m_device);
        swap(m_layout, other.m_layout);
        swap(m_pipeline, other.m_pipeline);

        return *this;
    }

private:
    VkDevice m_device = VK_NULL_HANDLE;
    VkPipelineLayout m_layout = VK_NULL_HANDLE;
    VkPipeline m_pipeline = VK_NULL_HANDLE;
};
}
