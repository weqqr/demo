#include <DM/Base.h>
#include <DM/Log.h>
#include <Demo/Renderer.h>
#include <Demo/Window.h>
#include <algorithm>
#include <vector>
#include <windows.h> // GetModuleHandle

#define VK_ASSERT(condition) ASSERT((condition) == VK_SUCCESS)

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
    VkFlags severity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
    VkFlags message_type = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

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

static VkDevice create_device(VkPhysicalDevice physical_device, QueueFamilies queue_families)
{
    std::vector<uint32_t> families = {
        queue_families.graphics,
        queue_families.compute,
        queue_families.present,
    };

    families.erase(std::unique(families.begin(), families.end()), families.end());

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

    VkDeviceCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
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

VkSwapchainKHR create_swapchain(VkPhysicalDevice physical_device, QueueFamilies queue_families, VkDevice device, VkSurfaceKHR surface, Size size)
{
    VkSurfaceCapabilitiesKHR capabilities = {};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &capabilities);

    VkExtent2D swapchain_extent = {
        .width = std::clamp(size.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
        .height = std::clamp(size.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height),
    };

    VkSwapchainCreateInfoKHR create_info = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = surface,
        .minImageCount = capabilities.minImageCount + 1,
        .imageFormat = VK_FORMAT_B8G8R8A8_SRGB,
        .imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR,
        .imageExtent = swapchain_extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 1,
        .pQueueFamilyIndices = &queue_families.present,
        .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = VK_PRESENT_MODE_FIFO_KHR,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE,
    };

    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    auto result = vkCreateSwapchainKHR(device, &create_info, nullptr, &swapchain);
    VK_ASSERT(result);

    return swapchain;
}

static VkCommandPool create_command_pool(VkDevice device, uint32_t family_index)
{
    VkCommandPoolCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = 0,
        .queueFamilyIndex = family_index,
    };

    VkCommandPool pool = VK_NULL_HANDLE;
    auto result = vkCreateCommandPool(device, &create_info, nullptr, &pool);
    VK_ASSERT(result);

    return pool;
}

Renderer::Renderer(const Window& window)
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
    vkGetDeviceQueue(m_device, m_queue_families.graphics, 0, &m_graphics);
    vkGetDeviceQueue(m_device, m_queue_families.compute, 0, &m_compute);
    vkGetDeviceQueue(m_device, m_queue_families.present, 0, &m_present);

    m_swapchain = create_swapchain(m_physical_device, m_queue_families, m_device, m_surface, window.size());

    uint32_t count = 0;
    vkGetSwapchainImagesKHR(m_device, m_swapchain, &count, nullptr);
    m_swapchain_images.resize(count);
    vkGetSwapchainImagesKHR(m_device, m_swapchain, &count, m_swapchain_images.data());

    for (auto image : m_swapchain_images) {
        VkImageViewCreateInfo create_info = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = image,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = VK_FORMAT_B8G8R8A8_SRGB,
            .components = {
                .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                .a = VK_COMPONENT_SWIZZLE_IDENTITY,
            },
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        };

        VkImageView view = VK_NULL_HANDLE;
        auto result = vkCreateImageView(m_device, &create_info, nullptr, &view);

        VK_ASSERT(result);

        m_swapchain_image_views.push_back(view);
    }

    m_command_pool = create_command_pool(m_device, m_queue_families.present);
}

Renderer::~Renderer()
{
    if (m_device) {
        vkDestroyCommandPool(m_device, m_command_pool, nullptr);

        for (auto view : m_swapchain_image_views) {
            vkDestroyImageView(m_device, view, nullptr);
        }

        vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
        vkDestroyDevice(m_device, nullptr);
    }

    if (m_instance) {
        vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
        vkDestroyDebugUtilsMessengerEXT(m_instance, m_debug_messenger, nullptr);
        vkDestroyInstance(m_instance, nullptr);
    }
}
}
