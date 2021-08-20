#include <Demo/Common/Base.h>
#include <Demo/Common/Log.h>
#include <Demo/Config.h>
#include <Demo/Renderer.h>
#include <Demo/Window.h>
#include <GLFW/glfw3.h>

#include <algorithm>
#include <array>
#include <fstream>
#include <optional>
#include <span>
#include <string_view>
#include <vector>

namespace Demo {
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

static VmaAllocator create_allocator(VkInstance instance, VkPhysicalDevice physical_device, VkDevice device)
{
    VmaVulkanFunctions vulkan_functions = {
        .vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties,
        .vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties,
        .vkAllocateMemory = vkAllocateMemory,
        .vkFreeMemory = vkFreeMemory,
        .vkMapMemory = vkMapMemory,
        .vkUnmapMemory = vkUnmapMemory,
        .vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges,
        .vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges,
        .vkBindBufferMemory = vkBindBufferMemory,
        .vkBindImageMemory = vkBindImageMemory,
        .vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements,
        .vkGetImageMemoryRequirements = vkGetImageMemoryRequirements,
        .vkCreateBuffer = vkCreateBuffer,
        .vkDestroyBuffer = vkDestroyBuffer,
        .vkCreateImage = vkCreateImage,
        .vkDestroyImage = vkDestroyImage,
        .vkCmdCopyBuffer = vkCmdCopyBuffer,
        .vkGetBufferMemoryRequirements2KHR = nullptr,
        .vkGetImageMemoryRequirements2KHR = nullptr,
        .vkBindBufferMemory2KHR = nullptr,
        .vkBindImageMemory2KHR = nullptr,
        .vkGetPhysicalDeviceMemoryProperties2KHR = nullptr,
    };

    VmaAllocatorCreateInfo create_info = {
        .physicalDevice = physical_device,
        .device = device,
        .pVulkanFunctions = &vulkan_functions,
        .instance = instance,
        .vulkanApiVersion = VK_API_VERSION_1_0,
    };

    VmaAllocator allocator = VK_NULL_HANDLE;
    auto result = vmaCreateAllocator(&create_info, &allocator);
    VK_ASSERT(result);

    return allocator;
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

static std::vector<uint8_t> load_binary_file(std::string_view path)
{
    std::ifstream ifs(path.data(), std::ios::binary);
    ASSERT(ifs.good());

    std::vector<uint8_t> buffer(std::istreambuf_iterator<char>(ifs), {});

    return buffer;
}

#pragma region memory

Buffer::Buffer(VmaAllocator allocator, size_t size, VkBufferUsageFlags buffer_usage, VmaMemoryUsage memory_usage)
{
    m_allocator = allocator;

    VkBufferCreateInfo buffer_create_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = buffer_usage,
    };

    VmaAllocationCreateInfo allocation_create_info = {
        .usage = memory_usage,
    };

    auto result = vmaCreateBuffer(allocator, &buffer_create_info, &allocation_create_info, &m_buffer, &m_allocation, nullptr);
    VK_ASSERT(result);
}

Buffer::~Buffer()
{
    if (m_allocator) {
        vmaDestroyBuffer(m_allocator, m_buffer, m_allocation);
    }
}

#pragma endregion

struct PushConstants {
    float time;
};

struct Uniforms {
    float time;
};

Renderer::Renderer(const Window& window)
    : RendererBase(window)
{
    m_size = window.size();
    m_swapchain = Swapchain(m_surface, m_physical_device, m_queue_families, m_device, m_size);
    m_command_pool = create_command_pool(m_device, m_queue_families.graphics);
    m_allocator = create_allocator(m_instance, m_physical_device, m_device);

    m_rendering_finished = create_semaphore(m_device);
    m_next_image_acquired = create_semaphore(m_device);
    m_gpu_work_finished = create_fence(m_device, false);

    std::array images = {
        RenderPassImage{
            .format = VK_FORMAT_B8G8R8A8_SRGB,
            .load_op = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .store_op = VK_ATTACHMENT_STORE_OP_STORE,
        },
    };

    RenderPass render_pass({
        .device = m_device,
        .size = m_size,
        .images = images,
    });

    std::array pool_sizes = {
        VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 16},
    };

    VkDescriptorPoolCreateInfo descriptor_pool_create_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = 16,
        .poolSizeCount = static_cast<uint32_t>(pool_sizes.size()),
        .pPoolSizes = pool_sizes.data(),
    };

    auto result = vkCreateDescriptorPool(m_device, &descriptor_pool_create_info, nullptr, &m_descriptor_pool);
    VK_ASSERT(result);

    VkDescriptorSetLayoutBinding binding = {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
    };

    VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = 1,
        .pBindings = &binding,
    };

    result = vkCreateDescriptorSetLayout(m_device, &descriptor_set_layout_create_info, nullptr, &m_descriptor_set_layout);
    VK_ASSERT(result);

    VkDescriptorSetAllocateInfo descriptor_set_allocate_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = m_descriptor_pool,
        .descriptorSetCount = 1,
        .pSetLayouts = &m_descriptor_set_layout,
    };

    result = vkAllocateDescriptorSets(m_device, &descriptor_set_allocate_info, &m_descriptor_set);
    VK_ASSERT(result);

    m_uniforms = Buffer(m_allocator, sizeof(Uniforms), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

    // Point descriptor to buffer
    VkDescriptorBufferInfo buffer_info = {
        .buffer = m_uniforms.raw(),
        .offset = 0,
        .range = sizeof(Uniforms),
    };

    VkWriteDescriptorSet write_descriptor_set = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = m_descriptor_set,
        .dstBinding = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .pBufferInfo = &buffer_info,
    };

    vkUpdateDescriptorSets(m_device, 1, &write_descriptor_set, 0, nullptr);

    auto vertex_spirv = load_binary_file("../Demo/Shaders/demo.vert.spv");
    auto fragment_spirv = load_binary_file("../Demo/Shaders/demo.frag.spv");

    GraphicsPipelineDesc desc{
        .device = m_device,
        .render_pass = render_pass,
        .descriptor_set_layouts = {
            m_descriptor_set_layout,
        },
        .push_constant_ranges = {
            {
                .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
                .offset = 0,
                .size = sizeof(PushConstants),
            },
        },
        .vertex_shader = Shader(m_device, vertex_spirv),
        .fragment_shader = Shader(m_device, fragment_spirv),
    };
    m_pipeline = GraphicsPipeline(desc);
}

Renderer::~Renderer()
{
    if (m_device) {
        dispose(m_pipeline);
        dispose(m_uniforms);
        // vkFreeDescriptorSets(m_device, m_descriptor_pool, 1, &m_descriptor_set);
        vkDestroyDescriptorSetLayout(m_device, m_descriptor_set_layout, nullptr);
        vkDestroyDescriptorPool(m_device, m_descriptor_pool, nullptr);

        vkDestroyFence(m_device, m_gpu_work_finished, nullptr);
        vkDestroySemaphore(m_device, m_next_image_acquired, nullptr);
        vkDestroySemaphore(m_device, m_rendering_finished, nullptr);

        vmaDestroyAllocator(m_allocator);
        vkDestroyCommandPool(m_device, m_command_pool, nullptr);

        dispose(m_swapchain);
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
    VK_ASSERT(result);

    VkCommandBufferBeginInfo cmd_begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    VK_ASSERT(vkBeginCommandBuffer(cmd, &cmd_begin_info));

    f(cmd);

    VK_ASSERT(vkEndCommandBuffer(cmd));

    return cmd;
}

void Renderer::resize(Size size)
{
    vkDeviceWaitIdle(m_device);
    m_size = size;

    if (size.rectangle_area() > 0) {
        dispose(m_swapchain);
        m_swapchain = Swapchain(m_surface, m_physical_device, m_queue_families, m_device, m_size);
    }
}

void Renderer::render()
{
    if (m_size.rectangle_area() == 0) {
        return;
    }

    std::array images = {
        RenderPassImage{
            .format = VK_FORMAT_B8G8R8A8_SRGB,
            .load_op = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .store_op = VK_ATTACHMENT_STORE_OP_STORE,
        },
    };

    RenderPass render_pass({
        .device = m_device,
        .size = m_size,
        .images = images,
    });

    auto [view, index] = m_swapchain.acquire_next_image(m_next_image_acquired);

    VkViewport viewport = {
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(m_size.width),
        .height = static_cast<float>(m_size.height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };

    VkRect2D scissor = {
        .offset = {0, 0},
        .extent = {m_size.width, m_size.height},
    };

    std::array clear_values = {
        VkClearValue{.color = {0.5f, 0.7f, 0.9f, 1.0f}},
    };
    std::array image_views = {view};

    PushConstants push_constants = {
        .time = static_cast<float>(glfwGetTime()),
    };

    Uniforms uniforms = {
        .time = static_cast<float>(glfwGetTime()),
    };

    m_uniforms.map([&](auto* ptr) {
        memcpy(ptr, &uniforms, sizeof(Uniforms));
    });

    auto cmd = record_command_buffer(m_device, m_command_pool, [&](auto cmd) {
        render_pass.execute(cmd, clear_values, image_views, [&]() {
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline.raw());
            vkCmdSetViewport(cmd, 0, 1, &viewport);
            vkCmdSetScissor(cmd, 0, 1, &scissor);
            vkCmdPushConstants(cmd, m_pipeline.layout(), VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstants), &push_constants);
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline.layout(), 0, 1, &m_descriptor_set, 0, nullptr);
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
        .pSwapchains = m_swapchain.as_ptr(),
        .pImageIndices = &index,
        .pResults = nullptr,
    };

    VK_ASSERT(vkQueuePresentKHR(m_present, &present_info));

    VK_ASSERT(vkWaitForFences(m_device, 1, &m_gpu_work_finished, VK_TRUE, TIMEOUT));
    vkResetFences(m_device, 1, &m_gpu_work_finished);

    vkFreeCommandBuffers(m_device, m_command_pool, 1, &cmd);
    vkResetCommandPool(m_device, m_command_pool, 0);
}
}
