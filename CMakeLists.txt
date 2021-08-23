cmake_minimum_required(VERSION 3.8)
project(Demo)

add_compile_definitions(VK_USE_PLATFORM_WIN32_KHR WIN32_LEAN_AND_MEAN NOMINMAX)

add_subdirectory(Vendor/fmt)
add_subdirectory(Vendor/glfw)
add_subdirectory(Vendor/vma)
add_subdirectory(Vendor/volk)

#    Shaders   #################################################################

file(GLOB SHADER_SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/Demo/Shaders/*.frag"
    "${CMAKE_CURRENT_LIST_DIR}/Demo/Shaders/*.vert"
)

file(GLOB SHADER_LIBRARIES
    "${CMAKE_CURRENT_LIST_DIR}/Demo/Shaders/*.glsl"
)

foreach(SHADER_PATH ${SHADER_SOURCES})
    get_filename_component(SHADER_FILE ${SHADER_PATH} NAME)
    set(SPIRV_PATH "${CMAKE_CURRENT_LIST_DIR}/Demo/Shaders/${SHADER_FILE}.spv")
    message(STATUS "glslangValidator -V ${SHADER_PATH} -o ${SPIRV_PATH} -I${SHADER_INCLUDE_PATH}")
    add_custom_command(
        OUTPUT ${SPIRV_PATH}
        COMMAND glslangValidator -V ${SHADER_PATH} -o ${SPIRV_PATH}
        DEPENDS ${SHADER_PATH} ${SHADER_LIBRARIES}
    )
    list(APPEND SPIRV_PATHS ${SPIRV_PATH})
endforeach()

add_custom_target(DemoShaders DEPENDS ${SPIRV_PATHS})

#   Demo   #####################################################################

add_executable(Demo
    Demo/Common/Base.cpp
    Demo/Common/Log.cpp
    Demo/Buffer.cpp
    Demo/Descriptor.cpp
    Demo/Main.cpp
    Demo/Mesh.cpp
    Demo/Pipeline.cpp
    Demo/Renderer.cpp
    Demo/RendererBase.cpp
    Demo/RenderPass.cpp
    Demo/Shader.cpp
    Demo/Swapchain.cpp
    Demo/Window.cpp)
add_dependencies(Demo DemoShaders)
target_compile_features(Demo PUBLIC cxx_std_20)
target_include_directories(Demo PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(Demo fmt glfw vma volk)
