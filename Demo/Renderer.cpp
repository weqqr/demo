#include <DM/Base.h>
#include <DM/Log.h>
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

#pragma region render pass
struct RenderPassImage {
    VkFormat format;
    VkAttachmentLoadOp load_op;
    VkAttachmentStoreOp store_op;
    std::optional<VkClearValue> clear_value;
};

struct RenderPassDesc {
    VkDevice device;
    Size size;
    std::span<RenderPassImage> images;
};

RenderPass::RenderPass(const RenderPassDesc& desc)
{
    ASSERT(desc.images.size() > 0);

    m_device = desc.device;
    m_size = desc.size;

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
            .width = desc.size.width,
            .height = desc.size.height,
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
        .width = desc.size.width,
        .height = desc.size.height,
        .layers = 1,
    };

    result = vkCreateFramebuffer(m_device, &fb_create_info, nullptr, &m_framebuffer);
    VK_ASSERT(result);
}

void RenderPass::begin_render_pass(VkCommandBuffer cmd, std::span<VkClearValue> clear_values, std::span<VkImageView> images)
{
    VkRenderPassAttachmentBeginInfo rp_attachment_begin_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_ATTACHMENT_BEGIN_INFO,
        .attachmentCount = static_cast<uint32_t>(images.size()),
        .pAttachments = images.data(),
    };

    VkRenderPassBeginInfo rp_begin_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .pNext = &rp_attachment_begin_info,
        .renderPass = m_render_pass,
        .framebuffer = m_framebuffer,
        .renderArea = {
            .offset = {0, 0},
            .extent = {m_size.width, m_size.height},
        },
        .clearValueCount = static_cast<uint32_t>(clear_values.size()),
        .pClearValues = clear_values.data(),
    };

    vkCmdBeginRenderPass(cmd, &rp_begin_info, VK_SUBPASS_CONTENTS_INLINE);
}

void RenderPass::end_render_pass(VkCommandBuffer cmd)
{
    vkCmdEndRenderPass(cmd);
}

#pragma endregion

#pragma region pipeline
struct PushConstants {
    float time;
};

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

static VkPipelineLayout create_pipeline_layout(VkDevice device)
{
    VkPushConstantRange push_constant_range = {
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
        .offset = 0,
        .size = sizeof(PushConstants),
    };

    VkPipelineLayoutCreateInfo layout_create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 0,
        .pSetLayouts = nullptr,
        .pushConstantRangeCount = 1,
        .pPushConstantRanges = &push_constant_range,
    };

    VkPipelineLayout layout = VK_NULL_HANDLE;
    auto result = vkCreatePipelineLayout(device, &layout_create_info, nullptr, &layout);
    VK_ASSERT(result);

    return layout;
}

static VkPipeline create_pipeline(VkDevice device, VkRenderPass render_pass, VkPipelineLayout layout)
{
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
        .width = 1.0f,
        .height = 1.0f,
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };

    VkRect2D scissor = {
        .offset = {0, 0},
        .extent = {1, 1},
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

    std::array dynamic_states = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
    };

    VkPipelineDynamicStateCreateInfo dynamic_state = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = static_cast<uint32_t>(dynamic_states.size()),
        .pDynamicStates = dynamic_states.data(),
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
        .pDynamicState = &dynamic_state,
        .layout = layout,
        .renderPass = render_pass,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = 0,
    };

    VkPipeline pipeline = VK_NULL_HANDLE;
    auto result = vkCreateGraphicsPipelines(device, nullptr, 1, &pipeline_create_info, nullptr, &pipeline);

    vkDestroyShaderModule(device, vertex_shader, nullptr);
    vkDestroyShaderModule(device, fragment_shader, nullptr);

    return pipeline;
}

#pragma endregion

#pragma region memory

Buffer::Buffer(VmaAllocator allocator, size_t size, VkBufferUsageFlags buffer_usage, VmaMemoryUsage memory_usage)
{
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

    m_layout = create_pipeline_layout(m_device);
    m_pipeline = create_pipeline(m_device, render_pass.raw(), m_layout);
}

Renderer::~Renderer()
{
    if (m_device) {
        vkDestroyPipeline(m_device, m_pipeline, nullptr);
        vkDestroyPipelineLayout(m_device, m_layout, nullptr);

        vkDestroyFence(m_device, m_gpu_work_finished, nullptr);
        vkDestroySemaphore(m_device, m_next_image_acquired, nullptr);
        vkDestroySemaphore(m_device, m_rendering_finished, nullptr);

        vmaDestroyAllocator(m_allocator);
        vkDestroyCommandPool(m_device, m_command_pool, nullptr);

        DM::dispose(m_swapchain);
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

void Renderer::resize(Size size)
{
    vkDeviceWaitIdle(m_device);
    m_size = size;

    if (size.rectangle_area() > 0) {
        DM::dispose(m_swapchain);
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

    auto cmd = record_command_buffer(m_device, m_command_pool, [&](auto cmd) {
        render_pass.execute(cmd, clear_values, image_views, [&]() {
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
            vkCmdSetViewport(cmd, 0, 1, &viewport);
            vkCmdSetScissor(cmd, 0, 1, &scissor);
            vkCmdPushConstants(cmd, m_layout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstants), &push_constants);
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
