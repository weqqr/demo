#pragma once

#include <Demo/Common/Types.h>
#include <volk.h>

#include <span>

namespace Demo {
class Shader : NonCopyable {
public:
    Shader() = default;
    Shader(VkDevice device, std::span<uint8_t> spirv_bytes);
    ~Shader();

    VkShaderModule raw() const { return m_module; }

    Shader(Shader&& other) noexcept
    {
        *this = move(other);
    }

    Shader& operator=(Shader&& other) noexcept
    {
        swap(m_device, other.m_device);
        swap(m_module, other.m_module);

        return *this;
    }

private:
    VkDevice m_device = VK_NULL_HANDLE;
    VkShaderModule m_module = VK_NULL_HANDLE;
};
}
