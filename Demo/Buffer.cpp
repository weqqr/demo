#include <Demo/Buffer.h>

namespace Demo {
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
}
