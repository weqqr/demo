#pragma once

#include <DM/Base.h>
#include <DM/Types.h>
#include <Demo/Window.h>
#include <span>
#include <vector>
#include <volk.h>
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#include <vk_mem_alloc.h>

namespace Demo {
struct QueueFamilies {
    uint32_t graphics = VK_QUEUE_FAMILY_IGNORED;
    uint32_t present = VK_QUEUE_FAMILY_IGNORED;
    uint32_t compute = VK_QUEUE_FAMILY_IGNORED;

    QueueFamilies() = default;
    QueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
};

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

class Swapchain : DM::NonCopyable {
public:
    Swapchain() = default;
    Swapchain(VkSurfaceKHR surface, VkPhysicalDevice physical_device, QueueFamilies queue_families, VkDevice device, Size size);
    ~Swapchain();

    std::pair<VkImageView, uint32_t> acquire_next_image(VkSemaphore semaphore);
    const VkSwapchainKHR* as_ptr() const { return &m_swapchain; }

    Swapchain(Swapchain&& other)
    {
        *this = move(other);
    }

    Swapchain& operator=(Swapchain&& other)
    {
        DM::swap(m_device, other.m_device);
        DM::swap(m_swapchain, other.m_swapchain);
        DM::swap(m_swapchain_images, other.m_swapchain_images);
        DM::swap(m_swapchain_image_views, other.m_swapchain_image_views);

        return *this;
    }

private:
    VkDevice m_device = VK_NULL_HANDLE;
    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;

    std::vector<VkImage> m_swapchain_images;
    std::vector<VkImageView> m_swapchain_image_views;
};

class Renderer : DM::NonCopyable {
public:
    explicit Renderer(const Window& window);
    ~Renderer();
    void render();
    void resize(Size size);

private:
    VkInstance m_instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT m_debug_messenger = VK_NULL_HANDLE;
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    VkPhysicalDevice m_physical_device = VK_NULL_HANDLE;
    QueueFamilies m_queue_families = {};
    VkDevice m_device = VK_NULL_HANDLE;
    VkQueue m_graphics = VK_NULL_HANDLE;
    VkQueue m_compute = VK_NULL_HANDLE;
    VkQueue m_present = VK_NULL_HANDLE;

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
