#include <DM/Base.h>
#include <Demo/Renderer.h>
#include <vector>
#include <volk.h>

#define VK_ASSERT(condition) ASSERT((condition) == VK_SUCCESS)

namespace Demo {
static VkInstance create_instance()
{
    std::vector<const char*> extensions = {
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
    };

    std::vector<const char*> layers = {
        "VK_LAYER_KHRONOS_validation",
    };

    VkApplicationInfo application_info = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .apiVersion = VK_API_VERSION_1_2,
    };

    VkInstanceCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &application_info,
        .enabledLayerCount = static_cast<uint32_t>(layers.size()),
        .ppEnabledLayerNames = layers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
        .ppEnabledExtensionNames = extensions.data(),
    };

    VkInstance instance = VK_NULL_HANDLE;
    auto result = vkCreateInstance(&create_info, nullptr, &instance);
    VK_ASSERT(result);

    return instance;
}

Renderer::Renderer(const Window& window)
{
    VK_ASSERT(volkInitialize());

    m_instance = create_instance();
    volkLoadInstance(m_instance);
}

Renderer::~Renderer()
{
    if (m_instance) {
        vkDestroyInstance(m_instance, nullptr);
    }
}

}
