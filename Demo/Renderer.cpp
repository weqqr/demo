#include <Demo/Common/Base.h>
#include <Demo/Common/Log.h>
#include <Demo/Config.h>
#include <Demo/Math.h>
#include <Demo/Renderer.h>
#include <Demo/Window.h>
#include <GLFW/glfw3.h>

#include <algorithm>
#include <array>
#include <fstream>
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

struct PushConstants {
    float time;
};

Renderer::Renderer(const Window& window, GraphicsPass pass, const Mesh& mesh)
    : RendererBase(window)
{
    m_size = window.size();
    m_swapchain = Swapchain(m_surface, m_physical_device, m_queue_families, m_device, m_size);
    m_command_pool = create_command_pool(m_device, m_queue_families.graphics);
    m_allocator = create_allocator(m_instance, m_physical_device, m_device);

    m_rendering_finished = create_semaphore(m_device);
    m_next_image_acquired = create_semaphore(m_device);
    m_gpu_work_finished = create_fence(m_device, false);

    m_descriptor_set_allocator = DescriptorSetAllocator(m_device);

    m_gpu_mesh = GPUMesh(m_allocator, mesh);

    std::vector<DescriptorBinding> bindings;
    for (auto uniform_buffer : pass.uniform_buffers) {
        Buffer buffer(m_allocator, uniform_buffer.buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
        m_descriptor_set_buffers.push_back(move(buffer));

        bindings.push_back({
            .binding = uniform_buffer.binding,
            .buffer_info = {
                .buffer = m_descriptor_set_buffers.back().raw(),
                .offset = 0,
                .range = uniform_buffer.buffer_size,
            },
            .descriptor_type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .stages = VK_SHADER_STAGE_FRAGMENT_BIT,
        });
    }

    m_descriptor_set = DescriptorSet(m_device, m_descriptor_set_allocator, bindings);

    auto mesh_vertex_spirv = load_binary_file("../Demo/Shaders/mesh.vert.spv");
    auto mesh_fragment_spirv = load_binary_file("../Demo/Shaders/mesh.frag.spv");
    auto vertex_spirv = load_binary_file("../Demo/Shaders/fullscreen.vert.spv");
    auto fragment_spirv = load_binary_file("../Demo/Shaders/fullscreen.frag.spv");

    m_pipeline = GraphicsPipeline({
        .device = m_device,
        .descriptor_set_layouts = {
            m_descriptor_set.layout(),
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
        .images = {VK_FORMAT_B8G8R8A8_SRGB},
    });

    m_mesh_pipeline = GraphicsPipeline({
        .device = m_device,
        .vertex_layout = Vertex::layout(),
        .vertex_shader = Shader(m_device, mesh_vertex_spirv),
        .fragment_shader = Shader(m_device, mesh_fragment_spirv),
        .images = {VK_FORMAT_B8G8R8A8_SRGB},
    });
}

Renderer::~Renderer()
{
    if (m_device) {
        dispose(m_mesh_pipeline);
        dispose(m_pipeline);
        dispose(m_descriptor_set);
        dispose(m_descriptor_set_allocator);
        dispose(m_descriptor_set_buffers);

        vkDestroyFence(m_device, m_gpu_work_finished, nullptr);
        vkDestroySemaphore(m_device, m_next_image_acquired, nullptr);
        vkDestroySemaphore(m_device, m_rendering_finished, nullptr);

        dispose(m_gpu_mesh);
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
    // Skip rendering when the window is minimized
    if (m_size.rectangle_area() == 0) {
        return;
    }

    auto [view, index] = m_swapchain.acquire_next_image(m_next_image_acquired);

    RenderPass render_pass({
        .device = m_device,
        .size = m_size,
        .images = {
            RenderPassImage{
                .format = VK_FORMAT_B8G8R8A8_SRGB,
                .load_op = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .store_op = VK_ATTACHMENT_STORE_OP_STORE,
                .clear_value = VkClearValue{.color = {0.5f, 0.7f, 0.9f, 1.0f}},
            },
        },
    });

    PushConstants push_constants = {
        .time = static_cast<float>(glfwGetTime()),
    };

    std::array image_views = {view};
    auto cmd = record_command_buffer(m_device, m_command_pool, [&](auto cmd) {
        render_pass.execute(cmd, image_views, [&]() {
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline.raw());
            vkCmdPushConstants(cmd, m_pipeline.layout(), VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstants), &push_constants);
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline.layout(), 0, 1, m_descriptor_set.as_ptr(), 0, nullptr);
            vkCmdDraw(cmd, 3, 1, 0, 0);

            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_mesh_pipeline.raw());
//
//            VkDeviceSize offset = 0;
//            vkCmdBindVertexBuffers(cmd, 0, 1, m_gpu_mesh.buffer().as_ptr(), &offset);
//            vkCmdDraw(cmd, m_gpu_mesh.vertex_count(), 1, 0, 0);
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
