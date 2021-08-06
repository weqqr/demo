#include <DM/Base.h>
#include <DM/Types.h>
#include <GLFW/glfw3.h>

using namespace DM;

namespace Demo {
struct Size {
    uint32_t width;
    uint32_t height;

    Size(uint32_t width, uint32_t height)
        : width(width)
        , height(height)
    {
    }

    uint32_t rect_area() const
    {
        return width * height;
    }
};

class Window : DM::NonCopyable {
public:
    Window(const char* title, int width, int height);
    ~Window();

    bool close_requested() const;

private:
    GLFWwindow* m_window = nullptr;
};

Window::Window(const char* title, int width, int height)
{
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    m_window = glfwCreateWindow(width, height, title, nullptr, nullptr);
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

void init()
{
    ASSERT(glfwInit());
}

void terminate()
{
    glfwTerminate();
}

void run()
{
    Window window("Demo", 1280, 720);

    while (!window.close_requested()) {
        glfwPollEvents();
    }
}
}

int main()
{
    using namespace Demo;

    init();
    run();
    terminate();

    return 0;
}
