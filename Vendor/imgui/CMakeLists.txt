project(imgui)
add_library(imgui STATIC
    imgui.cpp
    imgui_demo.cpp
    imgui_draw.cpp
    imgui_tables.cpp
    imgui_widgets.cpp
    backends/imgui_impl_glfw.cpp
    backends/imgui_impl_vulkan.cpp)
target_include_directories(imgui PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_compile_definitions(imgui PUBLIC IMGUI_IMPL_VULKAN_NO_PROTOTYPES)
target_link_libraries(imgui glfw volk)
