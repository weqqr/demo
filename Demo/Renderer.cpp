#include <DM/Base.h>
#include <DM/Log.h>
#include <Demo/Renderer.h>
#include <Demo/Window.h>
#include <windows.h> // GetModuleHandle

#include <algorithm>
#include <array>
#include <fstream>
#include <optional>
#include <span>
#include <string_view>
#include <vector>

#define VK_ASSERT(condition) ASSERT((condition) == VK_SUCCESS)

namespace Demo {
constexpr uint64_t TIMEOUT = 5'000'000'000; // 5 seconds (in nanoseconds)
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

static std::vector<uint32_t> unique_families(QueueFamilies queue_families)
{
    std::vector<uint32_t> families = {
        queue_families.graphics,
        queue_families.compute,
        queue_families.present,
    };

    std::sort(families.begin(), families.end());
    families.erase(std::unique(families.begin(), families.end()), families.end());

    return families;
}

static VkDevice create_device(VkPhysicalDevice physical_device, QueueFamilies queue_families)
{
    auto families = unique_families(queue_families);

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

VkSwapchainKHR create_swapchain(
    VkPhysicalDevice physical_device,
    QueueFamilies queue_families,
    VkDevice device,
    VkSurfaceKHR surface,
    Size size)
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

    auto families = unique_families(queue_families);

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

static VkSemaphore create_semaphore(VkDevice device)
{
    VkSemaphoreCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };

    VkSemaphore semaphore = VK_NULL_HANDLE;
    auto result = vkCreateSemaphore(device, &create_info, nullptr, &semaphore);
    VK_ASSERT(result);

    return semaphore;
}

static VkFence create_fence(VkDevice device, bool signaled = false)
{
    VkFenceCreateFlags flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;

    VkFenceCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = flags,
    };

    VkFence fence = VK_NULL_HANDLE;
    auto result = vkCreateFence(device, &create_info, nullptr, &fence);
    VK_ASSERT(result);

    return fence;
}

#pragma region render pass
struct RenderPassImage {
    VkFormat format;
    Size size;
    VkAttachmentLoadOp load_op;
    VkAttachmentStoreOp store_op;
    std::optional<VkClearValue> clear_value;
};

struct RenderPassDesc {
    VkDevice device;
    std::span<RenderPassImage> images;
};

RenderPass::RenderPass(const RenderPassDesc& desc)
{
    m_device = desc.device;

    std::vector<VkAttachmentDescription> attachments;
    std::vector<VkAttachmentReference> attachment_refs;
    std::vector<VkFramebufferAttachmentImageInfo> framebuffer_attachments;
    std::vector<VkClearValue> clear_values;

    for (auto image : desc.images) {
        VkAttachmentDescription attachment = {
            .format = image.format,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = image.load_op,
            .storeOp = image.store_op,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        };

        VkAttachmentReference attachment_ref = {
            .attachment = 0,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        };

        VkFramebufferAttachmentImageInfo fb_attachment_image_info = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENT_IMAGE_INFO,
            .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
            .width = image.size.width,
            .height = image.size.height,
            .layerCount = 1,
            .viewFormatCount = 1,
            .pViewFormats = &image.format,
        };

        attachments.push_back(attachment);
        attachment_refs.push_back(attachment_ref);
        framebuffer_attachments.push_back(fb_attachment_image_info);

        if (image.clear_value) {
            clear_values.push_back(*image.clear_value);
        }
    }

    VkSubpassDescription subpass = {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .inputAttachmentCount = 0,
        .pInputAttachments = nullptr,
        .colorAttachmentCount = static_cast<uint32_t>(attachment_refs.size()),
        .pColorAttachments = attachment_refs.data(),
        .pResolveAttachments = nullptr,
        .pDepthStencilAttachment = nullptr,
        .preserveAttachmentCount = 0,
        .pPreserveAttachments = nullptr,
    };

    VkRenderPassCreateInfo rp_create_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = static_cast<uint32_t>(attachments.size()),
        .pAttachments = attachments.data(),
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 0,
        .pDependencies = nullptr,
    };

    auto result = vkCreateRenderPass(m_device, &rp_create_info, nullptr, &m_render_pass);
    VK_ASSERT(result);

    VkFramebufferAttachmentsCreateInfo fb_attachments_create_info = {
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENTS_CREATE_INFO,
        .attachmentImageInfoCount = static_cast<uint32_t>(framebuffer_attachments.size()),
        .pAttachmentImageInfos = framebuffer_attachments.data(),
    };

    VkFramebufferCreateInfo fb_create_info = {
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .pNext = &fb_attachments_create_info,
        .flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT,
        .renderPass = m_render_pass,
        .attachmentCount = 1,
        .pAttachments = nullptr,
        .width = 1280,
        .height = 720,
        .layers = 1,
    };

    result = vkCreateFramebuffer(m_device, &fb_create_info, nullptr, &m_framebuffer);
    VK_ASSERT(result);
}
#pragma endregion

static VkShaderModule create_shader_module(VkDevice device, std::span<uint8_t> spirv_bytes)
{
    VkShaderModuleCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = spirv_bytes.size(),
        .pCode = reinterpret_cast<const uint32_t*>(spirv_bytes.data()),
    };

    VkShaderModule shader_module = VK_NULL_HANDLE;
    auto result = vkCreateShaderModule(device, &create_info, nullptr, &shader_module);
    VK_ASSERT(result);

    return shader_module;
}

static std::vector<uint8_t> load_binary_file(std::string_view path)
{
    std::ifstream ifs(path.data(), std::ios::binary);
    ASSERT(ifs.good());

    std::vector<uint8_t> buffer(std::istreambuf_iterator<char>(ifs), {});

    return buffer;
}

static VkPipeline create_pipeline(VkDevice device, VkRenderPass render_pass, Size size)
{
    VkPipelineLayoutCreateInfo layout_create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 0,
        .pSetLayouts = nullptr,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = nullptr,
    };
    VkPipelineLayout layout = VK_NULL_HANDLE;
    auto result = vkCreatePipelineLayout(device, &layout_create_info, nullptr, &layout);
    VK_ASSERT(result);

    auto vertex_spirv = load_binary_file("../Demo/Shaders/demo.vert.spv");
    auto fragment_spirv = load_binary_file("../Demo/Shaders/demo.frag.spv");

    debug("vertex shader size={}", vertex_spirv.size());
    debug("fragment shader size={}", fragment_spirv.size());

    auto vertex_shader = create_shader_module(device, vertex_spirv);
    auto fragment_shader = create_shader_module(device, fragment_spirv);

    VkPipelineShaderStageCreateInfo vertex_stage = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = vertex_shader,
        .pName = "main",
        .pSpecializationInfo = nullptr,
    };

    VkPipelineShaderStageCreateInfo fragment_stage = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = fragment_shader,
        .pName = "main",
        .pSpecializationInfo = nullptr,
    };

    std::array stages = {vertex_stage, fragment_stage};

    VkPipelineVertexInputStateCreateInfo vertex_input_state = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 0,
        .pVertexBindingDescriptions = nullptr,
        .vertexAttributeDescriptionCount = 0,
        .pVertexAttributeDescriptions = nullptr,
    };

    VkPipelineInputAssemblyStateCreateInfo input_assembly_state = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE,
    };

    VkViewport viewport = {
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(size.width),
        .height = static_cast<float>(size.height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };

    VkRect2D scissor = {
        .offset = {0, 0},
        .extent = {size.width, size.height},
    };

    VkPipelineViewportStateCreateInfo viewport_state = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .pViewports = &viewport,
        .scissorCount = 1,
        .pScissors = &scissor,
    };

    VkPipelineRasterizationStateCreateInfo rasterization_state = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_NONE,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .depthBiasConstantFactor = 0.0f,
        .depthBiasClamp = 0.0f,
        .depthBiasSlopeFactor = 0.0f,
        .lineWidth = 1.0f,
    };

    VkPipelineMultisampleStateCreateInfo multisample_state = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = VK_FALSE,
        .minSampleShading = 1.0f,
        .pSampleMask = nullptr,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable = VK_FALSE,
    };

    VkPipelineColorBlendAttachmentState color_blend_attachment = {
        .blendEnable = VK_FALSE,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
    };

    VkPipelineColorBlendStateCreateInfo color_blend_state = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY,
        .attachmentCount = 1,
        .pAttachments = &color_blend_attachment,
        .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f},
    };

    VkGraphicsPipelineCreateInfo pipeline_create_info = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = static_cast<uint32_t>(stages.size()),
        .pStages = stages.data(),
        .pVertexInputState = &vertex_input_state,
        .pInputAssemblyState = &input_assembly_state,
        .pTessellationState = nullptr,
        .pViewportState = &viewport_state,
        .pRasterizationState = &rasterization_state,
        .pMultisampleState = &multisample_state,
        .pDepthStencilState = nullptr,
        .pColorBlendState = &color_blend_state,
        .pDynamicState = nullptr,
        .layout = layout,
        .renderPass = render_pass,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = 0,
    };

    VkPipeline pipeline = VK_NULL_HANDLE;
    result = vkCreateGraphicsPipelines(device, nullptr, 1, &pipeline_create_info, nullptr, &pipeline);

    vkDestroyPipelineLayout(device, layout, nullptr);
    vkDestroyShaderModule(device, vertex_shader, nullptr);
    vkDestroyShaderModule(device, fragment_shader, nullptr);

    return pipeline;
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

    m_command_pool = create_command_pool(m_device, m_queue_families.graphics);

    m_rendering_finished = create_semaphore(m_device);
    m_next_image_acquired = create_semaphore(m_device);
    m_gpu_work_finished = create_fence(m_device, false);
    std::array images = {
        RenderPassImage{
            .format = VK_FORMAT_B8G8R8A8_SRGB,
            .size = {1280, 720},
            .load_op = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .store_op = VK_ATTACHMENT_STORE_OP_STORE,
        },
    };

    m_render_pass = RenderPass({
        .device = m_device,
        .images = images,
    });

    m_pipeline = create_pipeline(m_device, m_render_pass.raw(), Size(1280, 720));
}

template<typename T>
void drop(T t) { }

Renderer::~Renderer()
{
    if (m_device) {
        vkDestroyPipeline(m_device, m_pipeline, nullptr);

        drop(move(m_render_pass));

        vkDestroyFence(m_device, m_gpu_work_finished, nullptr);
        vkDestroySemaphore(m_device, m_next_image_acquired, nullptr);
        vkDestroySemaphore(m_device, m_rendering_finished, nullptr);

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

template<typename F>
VkCommandBuffer record_command_buffer(VkDevice device, VkCommandPool pool, F f)
{
    VkCommandBufferAllocateInfo cmd_allocate_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };

    VkCommandBuffer cmd = VK_NULL_HANDLE;
    auto result = vkAllocateCommandBuffers(device, &cmd_allocate_info, &cmd);

    VkCommandBufferBeginInfo cmd_begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    VK_ASSERT(vkBeginCommandBuffer(cmd, &cmd_begin_info));

    f(cmd);

    VK_ASSERT(vkEndCommandBuffer(cmd));

    return cmd;
}

void Renderer::render()
{
    uint32_t image_index = 0;
    VK_ASSERT(vkAcquireNextImageKHR(m_device, m_swapchain, TIMEOUT, m_next_image_acquired, VK_NULL_HANDLE, &image_index));

    std::array clear_values = {
        VkClearValue{.color = {0.5f, 0.7f, 0.9f, 1.0f}},
    };
    std::array image_views = {m_swapchain_image_views[image_index]};

    auto cmd = record_command_buffer(m_device, m_command_pool, [&](auto cmd) {
        m_render_pass.execute(cmd, clear_values, image_views, [&]() {
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
            vkCmdDraw(cmd, 3, 1, 0, 0);
        });
    });

    VkPipelineStageFlags stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &m_next_image_acquired,
        .pWaitDstStageMask = &stage_mask,
        .commandBufferCount = 1,
        .pCommandBuffers = &cmd,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &m_rendering_finished,
    };

    VK_ASSERT(vkQueueSubmit(m_graphics, 1, &submit_info, m_gpu_work_finished));

    VkPresentInfoKHR present_info = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &m_rendering_finished,
        .swapchainCount = 1,
        .pSwapchains = &m_swapchain,
        .pImageIndices = &image_index,
        .pResults = nullptr,
    };

    VK_ASSERT(vkQueuePresentKHR(m_present, &present_info));

    VK_ASSERT(vkWaitForFences(m_device, 1, &m_gpu_work_finished, VK_TRUE, TIMEOUT));
    vkResetFences(m_device, 1, &m_gpu_work_finished);

    vkFreeCommandBuffers(m_device, m_command_pool, 1, &cmd);
    vkResetCommandPool(m_device, m_command_pool, 0);
}
}
