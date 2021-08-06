#pragma once

#include <DM/Types.h>

struct GLFWwindow;

namespace Demo {
struct Size {
    uint32_t width;
    uint32_t height;

    Size(uint32_t width, uint32_t height)
        : width(width)
        , height(height)
    {
    }

    uint32_t rectangle_area() const
    {
        return width * height;
    }
};

class Window : DM::NonCopyable {
public:
    Window(const char* title, Size size);
    ~Window();

    bool close_requested() const;
    void* raw_handle() const;

private:
    GLFWwindow* m_window = nullptr;
};

}
