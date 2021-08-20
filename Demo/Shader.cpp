#include <Demo/RendererBase.h>
#include <Demo/Shader.h>

namespace Demo {
Shader::Shader(VkDevice device, std::span<uint8_t> spirv_bytes)
{
    m_device = device;

    VkShaderModuleCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = spirv_bytes.size(),
        .pCode = reinterpret_cast<const uint32_t*>(spirv_bytes.data()),
    };

    auto result = vkCreateShaderModule(device, &create_info, nullptr, &m_module);
    VK_ASSERT(result);
}

Shader::~Shader()
{
    if (m_device) {
        vkDestroyShaderModule(m_device, m_module, nullptr);
    }
}
}
