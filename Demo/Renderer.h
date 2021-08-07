#pragma once

#include <DM/Types.h>
#include <volk.h>

namespace Demo {
class Window;

struct QueueFamilies {
    uint32_t graphics = VK_QUEUE_FAMILY_IGNORED;
    uint32_t present = VK_QUEUE_FAMILY_IGNORED;
    uint32_t compute = VK_QUEUE_FAMILY_IGNORED;

    QueueFamilies() = default;
    QueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
};

class Renderer : DM::NonCopyable {
public:
    explicit Renderer(const Window& window);
    ~Renderer();

private:
    VkInstance m_instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT m_debug_messenger = VK_NULL_HANDLE;
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    VkPhysicalDevice m_physical_device = VK_NULL_HANDLE;
    QueueFamilies m_queue_families = {};
};
}
