#pragma once

#include <DM/Types.h>
#include <volk.h>

namespace Demo {
class Window;

class Renderer : DM::NonCopyable {
public:
    explicit Renderer(const Window& window);
    ~Renderer();

private:
    VkInstance m_instance = VK_NULL_HANDLE;
};
}
