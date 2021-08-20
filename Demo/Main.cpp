#include <Demo/Common/Base.h>
#include <Demo/Common/Types.h>
#include <Demo/Renderer.h>
#include <Demo/Window.h>
#include <GLFW/glfw3.h>

namespace Demo {
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
    Window window("Demo", {1280, 720});
    window.set_size_limits({320, 180}, SIZE_UNBOUNDED);

    Renderer renderer(window);
    window.set_resize_handler([&](Size size) {
        renderer.resize(size);
        renderer.render();
    });

    while (!window.close_requested() && !window.key_pressed(GLFW_KEY_ESCAPE)) {
        glfwWaitEventsTimeout(1.0 / 60.0);
        renderer.render();
    }
}
}

int main()
{
    Demo::init();
    Demo::run();
    Demo::terminate();

    return 0;
}
