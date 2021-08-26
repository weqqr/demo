#pragma once

#include <Demo/Math.h>
#include <functional>

struct GLFWwindow;

namespace Demo {
static constexpr Vector2u SIZE_UNBOUNDED = Vector2u(-1, -1);

class Window : NonCopyable {
    friend void resize_callback(GLFWwindow* window, int width, int height);
    friend void mouse_move_callback(GLFWwindow* window, double x, double y);
    friend void key_callback(GLFWwindow* window, int, int, int, int);

    using ResizeHandler = void(Vector2u);
    using MouseMoveHandler = void(float x, float y);
    using KeyHandler = void(int key, int scancode, int action, int mods);

public:
    Window(const char* title, Vector2u size);
    ~Window();

    bool close_requested() const;
    void* raw_handle() const;
    GLFWwindow* glfw_handle() const;
    bool key_pressed(int key) const;
    Vector2u size() const;

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

    void set_size_limits(Vector2u minimum, Vector2u maximum);

private:
    GLFWwindow* m_window = nullptr;
    std::function<ResizeHandler> m_resize_handler = [](Vector2u) {};
    std::function<MouseMoveHandler> m_mouse_move_handler = [](float, float) {};
    std::function<KeyHandler> m_key_handler = [](int, int, int, int) {};
};

}
