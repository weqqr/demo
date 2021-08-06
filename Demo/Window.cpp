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
}
