#pragma once

#include <Demo/RendererBase.h>

namespace Demo {
class DescriptorSetAllocator : NonCopyable {
public:
    DescriptorSetAllocator() = default;
    DescriptorSetAllocator(VkDevice device);
    ~DescriptorSetAllocator();
    VkDescriptorSet allocate(VkDescriptorSetLayout layout);
    void reset();
    VkDescriptorPool pool() const { return m_pool; };

    DescriptorSetAllocator(DescriptorSetAllocator&& other) noexcept
    {
        *this = move(other);
    }

    DescriptorSetAllocator& operator=(DescriptorSetAllocator&& other) noexcept
    {
        swap(m_device, other.m_device);
        swap(m_pool, other.m_pool);

        return *this;
    }

private:
    VkDevice m_device = VK_NULL_HANDLE;
    VkDescriptorPool m_pool = VK_NULL_HANDLE;
};

struct DescriptorBinding {
    uint32_t binding;
    VkDescriptorBufferInfo buffer_info;
    VkDescriptorType descriptor_type;
    VkShaderStageFlags stages;
};

class DescriptorSet : NonCopyable {
public:
    DescriptorSet() = default;
    DescriptorSet(VkDevice device, DescriptorSetAllocator& allocator, std::vector<DescriptorBinding> bindings);
    ~DescriptorSet();

    VkDescriptorSetLayout layout() const { return m_layout; }
    VkDescriptorSet raw() const { return m_set; }
    const VkDescriptorSet* as_ptr() const { return &m_set; }

    DescriptorSet(DescriptorSet&& other) noexcept
    {
        *this = move(other);
    }

    DescriptorSet& operator=(DescriptorSet&& other) noexcept
    {
        swap(m_device, other.m_device);
        swap(m_layout, other.m_layout);
        swap(m_set, other.m_set);

        return *this;
    }

private:
    VkDevice m_device = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_layout = VK_NULL_HANDLE;
    VkDescriptorSet m_set = VK_NULL_HANDLE;
};
}
