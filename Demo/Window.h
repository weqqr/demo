#pragma once

#include <Demo/Common/Types.h>
#include <functional>

struct GLFWwindow;

namespace Demo {
struct Size {
    uint32_t width;
    uint32_t height;

    constexpr Size(uint32_t width, uint32_t height)
        : width(width)
        , height(height)
    {
    }

    uint32_t rectangle_area() const
    {
        return width * height;
    }
};

static constexpr Size SIZE_UNBOUNDED = Size(-1, -1);

class Window : NonCopyable {
    friend void resize_callback(GLFWwindow* window, int width, int height);
    friend void mouse_move_callback(GLFWwindow* window, double x, double y);
    friend void key_callback(GLFWwindow* window, int, int, int, int);

    using ResizeHandler = void(Size);
    using MouseMoveHandler = void(float x, float y);
    using KeyHandler = void(int key, int scancode, int action, int mods);

public:
    Window(const char* title, Size size);
    ~Window();

    bool close_requested() const;
    void* raw_handle() const;
    GLFWwindow* glfw_handle() const;
    bool key_pressed(int key) const;
    Size size() const;

    void set_resize_handler(std::function<ResizeHandler> handler)
    {
        m_resize_handler = move(handler);
    }

    void set_mouse_move_handler(std::function<MouseMoveHandler> handler)
    {
        m_mouse_move_handler = move(handler);
    }

    void set_key_handler(std::function<KeyHandler> handler)
    {
        m_key_handler = move(handler);
    }

    void set_size_limits(Size minimum, Size maximum);

private:
    GLFWwindow* m_window = nullptr;
    std::function<ResizeHandler> m_resize_handler = [](Size) {};
    std::function<MouseMoveHandler> m_mouse_move_handler = [](float, float) {};
    std::function<KeyHandler> m_key_handler = [](int, int, int, int) {};
};

}
