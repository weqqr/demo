#include <Demo/RenderPass.h>

namespace Demo {
RenderPass::RenderPass(const RenderPassDesc& desc)
{
    ASSERT(desc.images.size() > 0);

    m_device = desc.device;
    m_size = desc.size;

    m_viewport = {
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(m_size.width),
        .height = static_cast<float>(m_size.height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };

    m_scissor = {
        .offset = {0, 0},
        .extent = {m_size.width, m_size.height},
    };

    std::vector<VkAttachmentDescription> attachments;
    std::vector<VkAttachmentReference> attachment_refs;
    std::vector<VkFramebufferAttachmentImageInfo> framebuffer_attachments;

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

        m_clear_values.push_back(image.clear_value);
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

void RenderPass::begin_render_pass(VkCommandBuffer cmd, std::span<VkImageView> images)
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
        .clearValueCount = static_cast<uint32_t>(m_clear_values.size()),
        .pClearValues = m_clear_values.data(),
    };

    vkCmdBeginRenderPass(cmd, &rp_begin_info, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdSetViewport(cmd, 0, 1, &m_viewport);
    vkCmdSetScissor(cmd, 0, 1, &m_scissor);
}

void RenderPass::end_render_pass(VkCommandBuffer cmd)
{
    vkCmdEndRenderPass(cmd);
}
}
