#pragma once

#include <DM/Base.h>
#include <Demo/Window.h>
#include <vector>
#include <volk.h>

#define VK_ASSERT(condition) ASSERT((condition) == VK_SUCCESS)

namespace Demo {
struct QueueFamilies {
    uint32_t graphics = VK_QUEUE_FAMILY_IGNORED;
    uint32_t present = VK_QUEUE_FAMILY_IGNORED;
    uint32_t compute = VK_QUEUE_FAMILY_IGNORED;

    QueueFamilies() = default;
    QueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);

    std::vector<uint32_t> unique() const;
};

class RendererBase : DM::NonCopyable {
public:
    RendererBase() = default;
    RendererBase(const Window& window);
    ~RendererBase();

protected:
    VkInstance m_instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT m_debug_messenger = VK_NULL_HANDLE;
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    VkPhysicalDevice m_physical_device = VK_NULL_HANDLE;
    QueueFamilies m_queue_families = {};
    VkDevice m_device = VK_NULL_HANDLE;
    VkQueue m_graphics = VK_NULL_HANDLE;
    VkQueue m_compute = VK_NULL_HANDLE;
    VkQueue m_present = VK_NULL_HANDLE;
};
}
