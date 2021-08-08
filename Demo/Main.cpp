#include <DM/Base.h>
#include <DM/Types.h>
#include <Demo/Renderer.h>
#include <Demo/Window.h>
#include <GLFW/glfw3.h>

using namespace DM;

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
    Renderer renderer(window);

    while (!window.close_requested() && !window.key_pressed(GLFW_KEY_ESCAPE)) {
        glfwPollEvents();
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
