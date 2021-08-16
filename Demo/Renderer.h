#pragma once

#include <DM/Base.h>
#include <DM/Types.h>
#include <Demo/RenderPass.h>
#include <Demo/RendererBase.h>
#include <Demo/Swapchain.h>
#include <Demo/Window.h>
#include <span>
#include <vector>
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#include <vk_mem_alloc.h>

namespace Demo {
class Buffer : DM::NonCopyable {
public:
    Buffer() = default;
    Buffer(VmaAllocator allocator, size_t size, VkBufferUsageFlags buffer_usage, VmaMemoryUsage memory_usage);
    ~Buffer();

    VkBuffer raw() const { return m_buffer; }

    template<typename F>
    void map(F f)
    {
        void* data = nullptr;

        vmaMapMemory(m_allocator, m_allocation, &data);
        f(data);
        vmaUnmapMemory(m_allocator, m_allocation);
    }

    Buffer(Buffer&& other) noexcept
    {
        *this = move(other);
    }

    Buffer& operator=(Buffer&& other) noexcept
    {
        DM::swap(m_allocator, other.m_allocator);
        DM::swap(m_buffer, other.m_buffer);
        DM::swap(m_allocation, other.m_allocation);

        return *this;
    }

private:
    VmaAllocator m_allocator = VK_NULL_HANDLE;
    VkBuffer m_buffer = VK_NULL_HANDLE;
    VmaAllocation m_allocation = VK_NULL_HANDLE;
};

class Pipeline : DM::NonCopyable {
public:
    Pipeline() = default;
    Pipeline(VkDevice device, const RenderPass& render_pass, VkDescriptorSetLayout descriptor_set_layout);
    ~Pipeline();

    VkPipelineLayout layout() const { return m_layout; }
    VkPipeline raw() const { return m_pipeline; }

    Pipeline(Pipeline&& other) noexcept
    {
        *this = move(other);
    }

    Pipeline& operator=(Pipeline&& other) noexcept
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

    VkDescriptorPool m_descriptor_pool = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_descriptor_set_layout = VK_NULL_HANDLE;
    VkDescriptorSet m_descriptor_set = VK_NULL_HANDLE;
    Buffer m_uniforms = {};

    Pipeline m_pipeline = {};

    Size m_size = {0, 0};
};
}
