cmake_minimum_required(VERSION 3.8)
project(Demo)

add_compile_definitions(VK_USE_PLATFORM_WIN32_KHR)

add_subdirectory(Vendor/fmt)
add_subdirectory(Vendor/glfw)
add_subdirectory(Vendor/volk)

#   DM   #######################################################################

add_library(DM
    DM/Base.cpp
    DM/Bitflags.h
    DM/Log.cpp
    DM/Traits.h
    DM/Types.h)
target_compile_features(DM PUBLIC cxx_std_20)
target_include_directories(DM PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(DM fmt::fmt)

#    Shaders   #################################################################

file(GLOB SHADER_SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/Demo/Shaders/*.frag"
    "${CMAKE_CURRENT_LIST_DIR}/Demo/Shaders/*.vert"
)

foreach(SHADER_PATH ${SHADER_SOURCES})
    get_filename_component(SHADER_FILE ${SHADER_PATH} NAME)
    set(SPIRV_PATH "${CMAKE_CURRENT_LIST_DIR}/Demo/Shaders/${SHADER_FILE}.spv")
    add_custom_command(
        OUTPUT ${SPIRV_PATH}
        COMMAND glslangValidator -V ${SHADER_PATH} -o ${SPIRV_PATH}
        DEPENDS ${SHADER_PATH}
    )
    list(APPEND SPIRV_PATHS ${SPIRV_PATH})
endforeach()

add_custom_target(DemoShaders DEPENDS ${SPIRV_PATHS})

#   Demo   #####################################################################

add_executable(Demo
    Demo/Main.cpp
    Demo/Renderer.cpp
    Demo/Window.cpp)
add_dependencies(Demo DemoShaders)
target_compile_features(Demo PUBLIC cxx_std_20)
target_include_directories(Demo PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(Demo DM glfw volk)
