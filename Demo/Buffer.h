#pragma once

#include <Demo/RendererBase.h>
#include <vk_mem_alloc.h>

namespace Demo {
class Buffer : NonCopyable {
public:
    Buffer() = default;
    Buffer(VmaAllocator allocator, size_t size, VkBufferUsageFlags buffer_usage, VmaMemoryUsage memory_usage);
    ~Buffer();

    VkBuffer raw() const { return m_buffer; }

    template<typename F>
    void map(F f)
    {
        void* data = nullptr;

        vmaMapMemory(m_allocator, m_allocation, &data);
        f(data);
        vmaUnmapMemory(m_allocator, m_allocation);
    }

    Buffer(Buffer&& other) noexcept
    {
        *this = move(other);
    }

    Buffer& operator=(Buffer&& other) noexcept
    {
        swap(m_allocator, other.m_allocator);
        swap(m_buffer, other.m_buffer);
        swap(m_allocation, other.m_allocation);

        return *this;
    }

private:
    VmaAllocator m_allocator = VK_NULL_HANDLE;
    VkBuffer m_buffer = VK_NULL_HANDLE;
    VmaAllocation m_allocation = VK_NULL_HANDLE;
};

}
