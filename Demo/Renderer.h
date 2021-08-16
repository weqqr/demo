#pragma once

#include <DM/Base.h>
#include <DM/Types.h>
#include <Demo/RendererBase.h>
#include <Demo/Swapchain.h>
#include <Demo/Window.h>
#include <span>
#include <vector>
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#include <vk_mem_alloc.h>

namespace Demo {
struct RenderPassDesc;

class RenderPass : DM::NonCopyable {
public:
    RenderPass() = default;
    RenderPass(const RenderPassDesc& desc);

    ~RenderPass()
    {
        if (m_device) {
            vkDestroyFramebuffer(m_device, m_framebuffer, nullptr);
            vkDestroyRenderPass(m_device, m_render_pass, nullptr);
        }
    }

    template<typename F>
    void execute(VkCommandBuffer cmd, std::span<VkClearValue> clear_values, std::span<VkImageView> images, F f)
    {
        begin_render_pass(cmd, clear_values, images);
        f();
        end_render_pass(cmd);
    }

    VkRenderPass raw() const { return m_render_pass; }

    RenderPass(RenderPass&& other) noexcept
    {
        *this = move(other);
    }

    RenderPass& operator=(RenderPass&& other) noexcept
    {
        DM::swap(m_device, other.m_device);
        DM::swap(m_framebuffer, other.m_framebuffer);
        DM::swap(m_render_pass, other.m_render_pass);
        DM::swap(m_size, other.m_size);
        return *this;
    }

private:
    void begin_render_pass(VkCommandBuffer cmd, std::span<VkClearValue> clear_values, std::span<VkImageView> images);
    void end_render_pass(VkCommandBuffer cmd);

    VkDevice m_device = VK_NULL_HANDLE;
    VkFramebuffer m_framebuffer = VK_NULL_HANDLE;
    VkRenderPass m_render_pass = VK_NULL_HANDLE;
    Size m_size = {0, 0};
};

class Buffer : DM::NonCopyable {
public:
    Buffer() = default;
    Buffer(VmaAllocator allocator, size_t size, VkBufferUsageFlags buffer_usage, VmaMemoryUsage memory_usage);
    ~Buffer();

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

    VkPipelineLayout m_layout = VK_NULL_HANDLE;
    VkPipeline m_pipeline = VK_NULL_HANDLE;

    Size m_size = {0, 0};
};
}
