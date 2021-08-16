#include <Demo/Config.h>
#include <Demo/Swapchain.h>
#include <algorithm>

namespace Demo {

VkSwapchainKHR create_swapchain(
    VkPhysicalDevice physical_device,
    QueueFamilies queue_families,
    VkDevice device,
    VkSurfaceKHR surface,
    Size size,
    VkSwapchainKHR old_swapchain = VK_NULL_HANDLE)
{
    VkSurfaceCapabilitiesKHR capabilities = {};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &capabilities);

    VkExtent2D swapchain_extent = {
        .width = std::clamp(
            size.width,
            capabilities.minImageExtent.width,
            capabilities.maxImageExtent.width),
        .height = std::clamp(
            size.height,
            capabilities.minImageExtent.height,
            capabilities.maxImageExtent.height),
    };

    auto families = queue_families.unique();

    auto sharing_mode = families.size() == 1 ? VK_SHARING_MODE_EXCLUSIVE
                                             : VK_SHARING_MODE_CONCURRENT;

    VkSwapchainCreateInfoKHR create_info = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = surface,
        .minImageCount = capabilities.minImageCount + 1,
        .imageFormat = VK_FORMAT_B8G8R8A8_SRGB,
        .imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR,
        .imageExtent = swapchain_extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = sharing_mode,
        .queueFamilyIndexCount = static_cast<uint32_t>(families.size()),
        .pQueueFamilyIndices = families.data(),
        .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = VK_PRESENT_MODE_FIFO_KHR,
        .clipped = VK_TRUE,
        .oldSwapchain = old_swapchain,
    };

    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    auto result = vkCreateSwapchainKHR(device, &create_info, nullptr, &swapchain);
    VK_ASSERT(result);

    return swapchain;
}

Swapchain::Swapchain(VkSurfaceKHR surface, VkPhysicalDevice physical_device, QueueFamilies queue_families, VkDevice device, Size size)
{
    m_device = device;
    m_swapchain = create_swapchain(physical_device, queue_families, device, surface, size);

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
}

Swapchain::~Swapchain()
{
    if (m_device) {
        for (auto view : m_swapchain_image_views) {
            vkDestroyImageView(m_device, view, nullptr);
        }

        vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
    }
}

std::pair<VkImageView, uint32_t> Swapchain::acquire_next_image(VkSemaphore semaphore)
{
    uint32_t image_index = 0;
    VK_ASSERT(vkAcquireNextImageKHR(m_device, m_swapchain, TIMEOUT, semaphore, VK_NULL_HANDLE, &image_index));

    return {m_swapchain_image_views[image_index], image_index};
}

}
