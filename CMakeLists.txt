cmake_minimum_required(VERSION 3.8)
project(Demo)

add_compile_definitions(VK_USE_PLATFORM_WIN32_KHR)

add_subdirectory(Vendor/fmt)
add_subdirectory(Vendor/glfw)
add_subdirectory(Vendor/volk)

add_library(DM
        DM/Base.cpp
        DM/Bitflags.h
        DM/Log.cpp
        DM/Traits.h
        DM/Types.h)
target_compile_features(DM PUBLIC cxx_std_20)
target_include_directories(DM PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(DM fmt::fmt)

add_executable(Demo
        Demo/Main.cpp
        Demo/Renderer.cpp
        Demo/Window.cpp)
target_compile_features(Demo PUBLIC cxx_std_20)
target_include_directories(Demo PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(Demo DM glfw volk)
