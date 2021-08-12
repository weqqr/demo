cmake_minimum_required(VERSION 3.8)
project(vma)

find_package(Vulkan QUIET)

add_library(vma STATIC vk_mem_alloc.cpp)
target_include_directories(vma PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(vma Vulkan::Vulkan)
