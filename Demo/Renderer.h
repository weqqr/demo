#pragma once

#include <Demo/Buffer.h>
#include <Demo/Common/Base.h>
#include <Demo/Common/Types.h>
#include <Demo/Descriptor.h>
#include <Demo/Mesh.h>
#include <Demo/Pipeline.h>
#include <Demo/RenderPass.h>
#include <Demo/RendererBase.h>
#include <Demo/Swapchain.h>
#include <Demo/Window.h>
#include <span>
#include <vector>
#include <vk_mem_alloc.h>

namespace Demo {
struct UniformBuffer {
    uint32_t binding;
    size_t buffer_size;
};

struct GraphicsPass {
    std::vector<UniformBuffer> uniform_buffers;
};

class Renderer : public RendererBase {
public:
    Renderer(const Window& window, GraphicsPass pass, const Mesh& mesh);
    ~Renderer();
    void render();
    void resize(Size size);

    template<typename T>
    void update(uint32_t index, T t)
    {
        m_descriptor_set_buffers[index].map([&](auto* ptr) {
            memcpy(ptr, &t, sizeof(T));
        });
    }

private:
    Swapchain m_swapchain = {};

    VkCommandPool m_command_pool = VK_NULL_HANDLE;

    VmaAllocator m_allocator = VK_NULL_HANDLE;

    VkSemaphore m_rendering_finished = VK_NULL_HANDLE;
    VkSemaphore m_next_image_acquired = VK_NULL_HANDLE;
    VkFence m_gpu_work_finished = VK_NULL_HANDLE;

    GPUMesh m_gpu_mesh = {};
    DescriptorSetAllocator m_descriptor_set_allocator = {};
    DescriptorSet m_descriptor_set = {};
    std::vector<Buffer> m_descriptor_set_buffers = {};

    GraphicsPipeline m_pipeline = {};
    GraphicsPipeline m_mesh_pipeline = {};

    Size m_size = {0, 0};
};
}
