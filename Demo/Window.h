#pragma once

#include <DM/Types.h>
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

class Window : DM::NonCopyable {
    friend void resize_callback(GLFWwindow* window, int width, int height);

public:
    Window(const char* title, Size size);
    ~Window();

    bool close_requested() const;
    void* raw_handle() const;
    bool key_pressed(int key) const;
    Size size() const;

    void set_resize_handler(std::function<void(Size)> handler)
    {
        m_resize_handler = handler;
    }

    void set_size_limits(Size minimum, Size maximum);

private:
    GLFWwindow* m_window = nullptr;
    std::function<void(Size)> m_resize_handler = [](Size) {};
};

}
