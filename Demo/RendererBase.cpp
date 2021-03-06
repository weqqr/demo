#include <Demo/Common/Log.h>
#include <Demo/RendererBase.h>
#include <windows.h>

#include <algorithm>
#include <vector>

namespace Demo {
static VkInstance create_instance()
{
    std::vector<const char*> extensions = {
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
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

VkBool32 VKAPI_PTR debug_messenger_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT types,
    const VkDebugUtilsMessengerCallbackDataEXT* data,
    void* user_data)
{
    switch (severity) {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        error("{}", data->pMessage);
        break;

    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
    default:
        warning("{}", data->pMessage);
    }

    return VK_FALSE;
}

static VkDebugUtilsMessengerEXT create_debug_messenger(VkInstance instance)
{
    VkFlags severity
        = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;

    VkFlags message_type
        = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

    VkDebugUtilsMessengerCreateInfoEXT create_info = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity = severity,
        .messageType = message_type,
        .pfnUserCallback = debug_messenger_callback,
        .pUserData = nullptr,
    };

    VkDebugUtilsMessengerEXT messenger = VK_NULL_HANDLE;
    auto result = vkCreateDebugUtilsMessengerEXT(instance, &create_info, nullptr, &messenger);
    VK_ASSERT(result);

    return messenger;
}

static VkSurfaceKHR create_surface(VkInstance instance, const Window& window)
{
    VkWin32SurfaceCreateInfoKHR create_info = {
        .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
        .hinstance = GetModuleHandleA(nullptr),
        .hwnd = static_cast<HWND>(window.raw_handle()),
    };

    VkSurfaceKHR surface = VK_NULL_HANDLE;
    auto result = vkCreateWin32SurfaceKHR(instance, &create_info, nullptr, &surface);
    VK_ASSERT(result);

    return surface;
}

QueueFamilies::QueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    uint32_t count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);
    std::vector<VkQueueFamilyProperties> properties(count);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, properties.data());

    for (uint32_t family_index = 0; family_index < properties.size(); family_index++) {
        const auto& family = properties[family_index];
        bool supports_graphics = family.queueFlags & VK_QUEUE_GRAPHICS_BIT;
        bool supports_compute = family.queueFlags & VK_QUEUE_COMPUTE_BIT;

        if (supports_graphics)
            graphics = family_index;

        if (supports_compute)
            compute = family_index;

        VkBool32 supports_surface = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, family_index, surface, &supports_surface);
        if (supports_surface) {
            present = family_index;
        }

        debug("Queue #{}: graphics={}, compute={}, present={}", family_index, supports_graphics, supports_compute, supports_surface);
    }

    debug("Chosen queue families: graphics={}, compute={}, present={}", graphics, compute, present);
}

static std::pair<VkPhysicalDevice, QueueFamilies> select_physical_device(VkInstance instance, VkSurfaceKHR surface)
{
    uint32_t count = 0;
    vkEnumeratePhysicalDevices(instance, &count, nullptr);
    std::vector<VkPhysicalDevice> devices(count);
    vkEnumeratePhysicalDevices(instance, &count, devices.data());

    ASSERT(count > 0);

    for (auto device : devices) {
        VkPhysicalDeviceProperties properties = {};
        vkGetPhysicalDeviceProperties(device, &properties);

        VkPhysicalDeviceMemoryProperties memory_properties = {};
        vkGetPhysicalDeviceMemoryProperties(device, &memory_properties);

        QueueFamilies queue_families(device, surface);

        bool has_graphics = queue_families.graphics != VK_QUEUE_FAMILY_IGNORED;
        bool has_compute = queue_families.compute != VK_QUEUE_FAMILY_IGNORED;
        bool has_present = queue_families.present != VK_QUEUE_FAMILY_IGNORED;

        if (!(has_graphics && has_compute && has_present)) {
            continue;
        }

        uint64_t total_memory = 0;
        for (auto heap : memory_properties.memoryHeaps) {
            if (heap.flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
                total_memory += heap.size;
            }
        }

        info("GPU: {}", properties.deviceName);
        info("GPU Memory: {} MiB", total_memory / 1024 / 1024);

        return {device, queue_families};
    }

    PANIC("No devices found");
}

std::vector<uint32_t> QueueFamilies::unique() const
{
    std::vector<uint32_t> families = {
        graphics,
        compute,
        present,
    };

    std::sort(families.begin(), families.end());
    families.erase(std::unique(families.begin(), families.end()), families.end());

    return families;
}

static VkDevice create_device(VkPhysicalDevice physical_device, QueueFamilies queue_families)
{
    auto families = queue_families.unique();

    std::vector<VkDeviceQueueCreateInfo> device_queue_create_infos;
    float priority = 1.0f;
    for (auto family_index : families) {
        VkDeviceQueueCreateInfo device_queue_create_info = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = family_index,
            .queueCount = 1,
            .pQueuePriorities = &priority,
        };

        device_queue_create_infos.push_back(device_queue_create_info);
    }

    std::vector<const char*> extensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };

    VkPhysicalDeviceVulkan12Features features_1_2 = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
        .imagelessFramebuffer = VK_TRUE,
    };

    VkDeviceCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &features_1_2,
        .queueCreateInfoCount = static_cast<uint32_t>(device_queue_create_infos.size()),
        .pQueueCreateInfos = device_queue_create_infos.data(),
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = nullptr,
        .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
        .ppEnabledExtensionNames = extensions.data(),
        .pEnabledFeatures = nullptr,
    };

    VkDevice device = VK_NULL_HANDLE;
    auto result = vkCreateDevice(physical_device, &create_info, nullptr, &device);
    VK_ASSERT(result);

    return device;
}

RendererBase::RendererBase(const Window& window)
{
    VK_ASSERT(volkInitialize());

    m_instance = create_instance();
    volkLoadInstance(m_instance);
    m_debug_messenger = create_debug_messenger(m_instance);
    m_surface = create_surface(m_instance, window);
    auto [physical_device, queue_families] = select_physical_device(m_instance, m_surface);
    m_physical_device = physical_device;
    m_queue_families = queue_families;
    m_device = create_device(m_physical_device, m_queue_families);
    volkLoadDevice(m_device);

    vkGetDeviceQueue(m_device, m_queue_families.graphics, 0, &m_graphics);
    vkGetDeviceQueue(m_device, m_queue_families.compute, 0, &m_compute);
    vkGetDeviceQueue(m_device, m_queue_families.present, 0, &m_present);
}

RendererBase::~RendererBase()
{
    if (m_device) {
        vkDestroyDevice(m_device, nullptr);
    }

    if (m_instance) {
        vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
        vkDestroyDebugUtilsMessengerEXT(m_instance, m_debug_messenger, nullptr);
        vkDestroyInstance(m_instance, nullptr);
    }
}
}
