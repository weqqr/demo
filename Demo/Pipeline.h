#pragma once

#include <Demo/RendererBase.h>
#include <Demo/RenderPass.h>
#include <Demo/Shader.h>
#include <vector>

namespace Demo {
struct GraphicsPipelineDesc {
    VkDevice device;
    const RenderPass& render_pass;
    std::vector<VkDescriptorSetLayout> descriptor_set_layouts;
    std::vector<VkPushConstantRange> push_constant_ranges;

    Shader vertex_shader;
    Shader fragment_shader;
};

class GraphicsPipeline : DM::NonCopyable {
public:
    GraphicsPipeline() = default;
    GraphicsPipeline(GraphicsPipelineDesc& desc);
    ~GraphicsPipeline();

    VkPipelineLayout layout() const { return m_layout; }
    VkPipeline raw() const { return m_pipeline; }

    GraphicsPipeline(GraphicsPipeline&& other) noexcept
    {
        *this = move(other);
    }

    GraphicsPipeline& operator=(GraphicsPipeline&& other) noexcept
    {
        DM::swap(m_device, other.m_device);
        DM::swap(m_layout, other.m_layout);
        DM::swap(m_pipeline, other.m_pipeline);

        return *this;
    }

private:
    VkDevice m_device = VK_NULL_HANDLE;
    VkPipelineLayout m_layout = VK_NULL_HANDLE;
    VkPipeline m_pipeline = VK_NULL_HANDLE;
};
}
