#include <Demo/Common/Log.h>
#include <Demo/Pipeline.h>

#include <algorithm>
#include <array>

namespace Demo {
static VkPipelineLayout create_pipeline_layout(
    VkDevice device,
    const std::vector<VkDescriptorSetLayout>& descriptor_set_layouts,
    const std::vector<VkPushConstantRange>& push_constant_ranges)
{
    VkPipelineLayoutCreateInfo layout_create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = static_cast<uint32_t>(descriptor_set_layouts.size()),
        .pSetLayouts = descriptor_set_layouts.data(),
        .pushConstantRangeCount = static_cast<uint32_t>(push_constant_ranges.size()),
        .pPushConstantRanges = push_constant_ranges.data(),
    };

    VkPipelineLayout layout = VK_NULL_HANDLE;
    auto result = vkCreatePipelineLayout(device, &layout_create_info, nullptr, &layout);
    VK_ASSERT(result);

    return layout;
}

static VkPipeline create_pipeline(VkDevice device, std::optional<VertexLayout> vertex_layout, VkRenderPass render_pass, VkPipelineLayout layout, Shader vertex, Shader fragment)
{
    VkPipelineShaderStageCreateInfo vertex_stage = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = vertex.raw(),
        .pName = "main",
        .pSpecializationInfo = nullptr,
    };

    VkPipelineShaderStageCreateInfo fragment_stage = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = fragment.raw(),
        .pName = "main",
        .pSpecializationInfo = nullptr,
    };

    std::array stages = {vertex_stage, fragment_stage};

    VkPipelineVertexInputStateCreateInfo vertex_input_state;
    if (vertex_layout) {
        vertex_input_state = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .vertexBindingDescriptionCount = static_cast<uint32_t>(vertex_layout->binding_descriptions.size()),
            .pVertexBindingDescriptions = vertex_layout->binding_descriptions.data(),
            .vertexAttributeDescriptionCount = static_cast<uint32_t>(vertex_layout->attribute_descriptions.size()),
            .pVertexAttributeDescriptions = vertex_layout->attribute_descriptions.data(),
        };
    } else {
        vertex_input_state = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        };
    }

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
    VK_ASSERT(result);

    return pipeline;
}

GraphicsPipeline::GraphicsPipeline(GraphicsPipelineDesc desc)
{
    m_device = desc.device;

    std::vector<RenderPassImage> images;
    std::transform(desc.images.begin(), desc.images.end(), std::back_inserter(images), [](VkFormat format) {
        return RenderPassImage{
            .format = format,
            .load_op = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .store_op = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        };
    });

    RenderPass render_pass({
        .device = m_device,
        .size = Vector2u(1, 1),
        .images = images,
    });

    m_layout = create_pipeline_layout(m_device, desc.descriptor_set_layouts, desc.push_constant_ranges);
    m_pipeline = create_pipeline(m_device, desc.vertex_layout, render_pass.raw(), m_layout, move(desc.vertex_shader), move(desc.fragment_shader));
}

GraphicsPipeline::~GraphicsPipeline()
{
    if (m_device) {
        vkDestroyPipeline(m_device, m_pipeline, nullptr);
        vkDestroyPipelineLayout(m_device, m_layout, nullptr);
    }
}
}
