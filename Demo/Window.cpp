#include <Demo/Common/Base.h>
#include <Demo/Window.h>
#include <GLFW/glfw3.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

namespace Demo {
static void resize_callback(GLFWwindow* window, int width, int height)
{
    auto* w = static_cast<Window*>(glfwGetWindowUserPointer(window));

    w->m_resize_handler(Size(static_cast<uint32_t>(width), static_cast<uint32_t>(height)));
}

Window::Window(const char* title, Size size)
{
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    m_window = glfwCreateWindow(static_cast<int>(size.width), static_cast<int>(size.height), title, nullptr, nullptr);

    ASSERT(m_window != nullptr);

    glfwSetWindowUserPointer(m_window, this);
    glfwSetWindowSizeCallback(m_window, resize_callback);
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

    return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
}

void Window::set_size_limits(Size minimum, Size maximum)
{
    glfwSetWindowSizeLimits(m_window, minimum.width, minimum.height, maximum.width, maximum.height);
}
}
