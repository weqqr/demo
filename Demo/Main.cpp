#include <Demo/Common/Base.h>
#include <Demo/Common/Types.h>
#include <Demo/Math.h>
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

struct Uniforms {
    Vector4 position;
    Vector4 look_dir;
    float time;
    float aspect_ratio;
    float fov;
    float width;
    float height;
};

void run()
{
    Window window("Demo", {1280, 720});
    window.set_size_limits({320, 180}, SIZE_UNBOUNDED);

    GraphicsPass pass{
        .uniform_buffers = {
            UniformBuffer{
                .binding = 0,
                .buffer_size = sizeof(Uniforms),
            },
        },
    };

    Renderer renderer(window, pass);
    window.set_resize_handler([&](Size size) {
        renderer.resize(size);
        renderer.render();
    });

    while (!window.close_requested() && !window.key_pressed(GLFW_KEY_ESCAPE)) {
        glfwPollEvents();

        Uniforms uniforms = {
            .position = Vector4(Vector3(-1.0f, -1.0f, -1.0f), 0.0f),
            .look_dir = Vector4(Vector3(1.0f, 1.0f, 0.9f), 0.0f),
            .time = static_cast<float>(glfwGetTime()),
            .aspect_ratio = static_cast<float>(1280) / static_cast<float>(720),
            .fov = 90.0f,
            .width = static_cast<float>(1280),
            .height = static_cast<float>(720),
        };

        renderer.update(0, uniforms);
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
