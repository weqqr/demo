#pragma once

#include <Demo/RendererBase.h>
#include <vector>

namespace Demo {
class Swapchain : DM::NonCopyable {
public:
    Swapchain() = default;
    Swapchain(VkSurfaceKHR surface, VkPhysicalDevice physical_device, QueueFamilies queue_families, VkDevice device, Size size);
    ~Swapchain();

    std::pair<VkImageView, uint32_t> acquire_next_image(VkSemaphore semaphore);
    const VkSwapchainKHR* as_ptr() const { return &m_swapchain; }

    Swapchain(Swapchain&& other) noexcept
    {
        *this = move(other);
    }

    Swapchain& operator=(Swapchain&& other) noexcept
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
}
