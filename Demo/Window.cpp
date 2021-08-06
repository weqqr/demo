#include <DM/Base.h>
#include <Demo/Window.h>
#include <GLFW/glfw3.h>

namespace Demo {
Window::Window(const char* title, Size size)
{
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    m_window = glfwCreateWindow(size.width, size.height, title, nullptr, nullptr);

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
}
