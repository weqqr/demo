#include <Demo/Descriptor.h>

#include <array>

namespace Demo {
DescriptorSetAllocator::DescriptorSetAllocator(VkDevice device)
{
    m_device = device;

    std::array pool_sizes = {
        VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 16},
        VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 16},
    };

    VkDescriptorPoolCreateInfo descriptor_pool_create_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = 16,
        .poolSizeCount = static_cast<uint32_t>(pool_sizes.size()),
        .pPoolSizes = pool_sizes.data(),
    };

    auto result = vkCreateDescriptorPool(m_device, &descriptor_pool_create_info, nullptr, &m_pool);
    VK_ASSERT(result);
}

DescriptorSetAllocator::~DescriptorSetAllocator()
{
    if (m_device) {
        vkDestroyDescriptorPool(m_device, m_pool, nullptr);
    }
}

VkDescriptorSet DescriptorSetAllocator::allocate(VkDescriptorSetLayout layout)
{
    VkDescriptorSetAllocateInfo descriptor_set_allocate_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = m_pool,
        .descriptorSetCount = 1,
        .pSetLayouts = &layout,
    };

    VkDescriptorSet descriptor_set = VK_NULL_HANDLE;
    auto result = vkAllocateDescriptorSets(m_device, &descriptor_set_allocate_info, &descriptor_set);
    VK_ASSERT(result);

    return descriptor_set;
}

void DescriptorSetAllocator::reset()
{
    VK_ASSERT(vkResetDescriptorPool(m_device, m_pool, 0));
}

DescriptorSet::DescriptorSet(VkDevice device, DescriptorSetAllocator& allocator, std::vector<DescriptorBinding> bindings)
{
    m_device = device;

    std::vector<VkDescriptorSetLayoutBinding> descriptor_set_layout_bindings;

    for (auto binding : bindings) {
        VkDescriptorSetLayoutBinding descriptor_set_layout_binding = {
            .binding = binding.binding,
            .descriptorType = binding.descriptor_type,
            .descriptorCount = 1,
            .stageFlags = binding.stages,
        };

        descriptor_set_layout_bindings.push_back(descriptor_set_layout_binding);
    }

    VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = static_cast<uint32_t>(descriptor_set_layout_bindings.size()),
        .pBindings = descriptor_set_layout_bindings.data(),
    };

    auto result = vkCreateDescriptorSetLayout(m_device, &descriptor_set_layout_create_info, nullptr, &m_layout);
    VK_ASSERT(result);

    m_set = allocator.allocate(m_layout);

    std::vector<VkWriteDescriptorSet> writes;
    for (const auto& binding : bindings) {
        VkWriteDescriptorSet write_descriptor_set = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = m_set,
            .dstBinding = binding.binding,
            .descriptorCount = 1,
            .descriptorType = binding.descriptor_type,
            .pBufferInfo = &binding.buffer_info,
        };

        writes.push_back(write_descriptor_set);
    }

    vkUpdateDescriptorSets(m_device, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
}

DescriptorSet::~DescriptorSet()
{
    if (m_device) {
        vkDestroyDescriptorSetLayout(m_device, m_layout, nullptr);
    }
}

}
