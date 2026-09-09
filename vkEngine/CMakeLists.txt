cmake_minimum_required(VERSION 4.0)
project(vkEG)


file(GLOB_RECURSE srcs CONFIGURE_DEPENDS src/*.cpp include/*.hpp include/*.h)


add_executable(${PROJECT_NAME} ${srcs})

#链接glfw
target_link_libraries(${PROJECT_NAME} PRIVATE Vulkan::Vulkan glfw)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)