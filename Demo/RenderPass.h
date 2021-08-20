#pragma once

#include <Demo/RendererBase.h>
#include <optional>
#include <span>

namespace Demo {
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

class RenderPass : NonCopyable {
public:
    RenderPass() = default;
    RenderPass(const RenderPassDesc& desc);

    ~RenderPass()
    {
        if (m_device) {
            vkDestroyFramebuffer(m_device, m_framebuffer, nullptr);
            vkDestroyRenderPass(m_device, m_render_pass, nullptr);
        }
    }

    template<typename F>
    void execute(VkCommandBuffer cmd, std::span<VkClearValue> clear_values, std::span<VkImageView> images, F f)
    {
        begin_render_pass(cmd, clear_values, images);
        f();
        end_render_pass(cmd);
    }

    VkRenderPass raw() const { return m_render_pass; }

    RenderPass(RenderPass&& other) noexcept
    {
        *this = move(other);
    }

    RenderPass& operator=(RenderPass&& other) noexcept
    {
        swap(m_device, other.m_device);
        swap(m_framebuffer, other.m_framebuffer);
        swap(m_render_pass, other.m_render_pass);
        swap(m_size, other.m_size);
        return *this;
    }

private:
    void begin_render_pass(VkCommandBuffer cmd, std::span<VkClearValue> clear_values, std::span<VkImageView> images);
    void end_render_pass(VkCommandBuffer cmd);

    VkDevice m_device = VK_NULL_HANDLE;
    VkFramebuffer m_framebuffer = VK_NULL_HANDLE;
    VkRenderPass m_render_pass = VK_NULL_HANDLE;
    Size m_size = {0, 0};
};
}
