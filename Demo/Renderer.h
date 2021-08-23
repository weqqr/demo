#pragma once

#include <Demo/Buffer.h>
#include <Demo/Common/Base.h>
#include <Demo/Common/Types.h>
#include <Demo/Descriptor.h>
#include <Demo/Pipeline.h>
#include <Demo/RenderPass.h>
#include <Demo/RendererBase.h>
#include <Demo/Swapchain.h>
#include <Demo/Window.h>
#include <span>
#include <vector>
#include <vk_mem_alloc.h>

namespace Demo {
class Renderer : public RendererBase {
public:
    explicit Renderer(const Window& window);
    ~Renderer();
    void render();
    void resize(Size size);

private:
    Swapchain m_swapchain = {};

    VkCommandPool m_command_pool = VK_NULL_HANDLE;

    VmaAllocator m_allocator = VK_NULL_HANDLE;

    VkSemaphore m_rendering_finished = VK_NULL_HANDLE;
    VkSemaphore m_next_image_acquired = VK_NULL_HANDLE;
    VkFence m_gpu_work_finished = VK_NULL_HANDLE;


    DescriptorSetAllocator m_descriptor_set_allocator = {};
    DescriptorSet m_descriptor_set = {};
    Buffer m_uniforms = {};

    GraphicsPipeline m_pipeline = {};

    Size m_size = {0, 0};
};
}
