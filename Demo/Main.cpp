#include <Demo/Common/Base.h>
#include <Demo/Common/Log.h>
#include <Demo/Common/Types.h>
#include <Demo/FlyCamera.h>
#include <Demo/Math.h>
#include <Demo/Mesh.h>
#include <Demo/Renderer.h>
#include <Demo/Window.h>
#include <GLFW/glfw3.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
#include <imgui.h>

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
    float time;
    float aspect_ratio;
    float fov;
    float width;
    float height;
};

struct Camera {
    Vector4 position;
    Vector4 look_dir;
};

enum class InteractionMode {
    UI,
    Camera,
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
            UniformBuffer{
                .binding = 1,
                .buffer_size = sizeof(Camera),
            },
        },
    };

    Mesh mesh;
    mesh.add_vertex({{-0.5f, 0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}});
    mesh.add_vertex({{0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}});
    mesh.add_vertex({{0.0f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}});

    auto& imgui_io = ImGui::GetIO();
    imgui_io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForVulkan(window.glfw_handle(), true);

    Renderer renderer(window, pass, mesh);

    FlyCamera fly_camera({-1.0f, -1.0f, -1.0f}, 0.1f, 0.2f);
    InteractionMode mode = InteractionMode::Camera;

    window.set_resize_handler([&](Size size) {
        renderer.resize(size);
        renderer.render();
    });

    window.set_mouse_move_handler([&](float x, float y) {
        static float cx = x;
        static float cy = y;

        auto dx = cx - x;
        auto dy = cy - y;

        cx = x;
        cy = y;

        if (mode == InteractionMode::Camera) {
            fly_camera.rotate(dx, dy);
        }
    });

    window.set_key_handler([&](int key, int scancode, int action, int mods) {
        if (key == GLFW_KEY_GRAVE_ACCENT && action == GLFW_RELEASE) {
            if (mode == InteractionMode::Camera) {
                mode = InteractionMode::UI;
                glfwSetInputMode(window.glfw_handle(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            } else if (mode == InteractionMode::UI) {
                mode = InteractionMode::Camera;
                glfwSetInputMode(window.glfw_handle(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            }
        }
    });

    float fov = 90.0f;

    while (!window.close_requested()) {
        glfwPollEvents();

        ImGui_ImplGlfw_NewFrame();
        ImGui_ImplVulkan_NewFrame();
        ImGui::NewFrame();

        if (mode == InteractionMode::UI) {
            ImGui::Begin("Settings", nullptr);
            ImGui::SliderFloat("FOV", &fov, 40.0f, 140.0f, "%.0f", ImGuiSliderFlags_AlwaysClamp);
            if (ImGui::Button("Close Me")) {
                debug("button");
            }
            ImGui::End();
        } else {
            if (window.key_pressed(GLFW_KEY_ESCAPE))
                break;
            if (window.key_pressed(GLFW_KEY_W))
                fly_camera.move(MovementDirection::Forward);
            if (window.key_pressed(GLFW_KEY_S))
                fly_camera.move(MovementDirection::Backward);
            if (window.key_pressed(GLFW_KEY_A))
                fly_camera.move(MovementDirection::Left);
            if (window.key_pressed(GLFW_KEY_D))
                fly_camera.move(MovementDirection::Right);
            if (window.key_pressed(GLFW_KEY_SPACE))
                fly_camera.move(MovementDirection::Up);
            if (window.key_pressed(GLFW_KEY_LEFT_SHIFT))
                fly_camera.move(MovementDirection::Down);
        }

        Uniforms uniforms = {
            .time = static_cast<float>(glfwGetTime()),
            .aspect_ratio = static_cast<float>(1280) / static_cast<float>(720),
            .fov = fov,
            .width = static_cast<float>(1280),
            .height = static_cast<float>(720),
        };

        Camera camera = {
            .position = Vector4(fly_camera.position(), 0.0f),
            .look_dir = Vector4(fly_camera.look_dir(), 0.0f),
        };

        renderer.update(0, uniforms);
        renderer.update(1, camera);
        renderer.render();
    }

    ImGui_ImplGlfw_Shutdown();
}
}

int main()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    Demo::init();
    Demo::run();
    Demo::terminate();
    ImGui::DestroyContext();

    return 0;
}
