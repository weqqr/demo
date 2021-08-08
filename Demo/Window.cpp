#include <DM/Base.h>
#include <Demo/Window.h>
#include <GLFW/glfw3.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

namespace Demo {
Window::Window(const char* title, Size size)
{
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    m_window = glfwCreateWindow(static_cast<int>(size.width), static_cast<int>(size.height), title, nullptr, nullptr);

    ASSERT(m_window != nullptr);
}

Window::~Window()
{
    if (m_window) {
        glfwDestroyWindow(m_window);
    }
}

bool Window::close_requested() const
{
    return glfwWindowShouldClose(m_window);
}

void* Window::raw_handle() const
{
    return glfwGetWin32Window(m_window);
}

bool Window::key_pressed(int key) const
{
    return glfwGetKey(m_window, key) == GLFW_PRESS;
}

Size Window::size() const
{
    int width = 0;
    int height = 0;

    glfwGetFramebufferSize(m_window, &width, &height);

    ASSERT(width >= 0 && height >= 0);

    return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
}
}
