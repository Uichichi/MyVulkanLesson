cmake_minimum_required(VERSION 4.0)
project(vkEG)


file(GLOB_RECURSE srcs CONFIGURE_DEPENDS src/*.cpp include/*.hpp include/*.h)


add_executable(${PROJECT_NAME} ${srcs})

#自动编译spv
find_program(GLSLC_PROGRAM glslc REQUIRED)
set(shader_output_dir "${CMAKE_CURRENT_BINARY_DIR}/shaders/$<CONFIG>")
foreach(shader IN ITEMS simple_shader.vert simple_shader.frag)
    set(shader_source "${CMAKE_CURRENT_SOURCE_DIR}/shaders/${shader}")
    set(shader_output "${shader_output_dir}/${shader}.spv")
    add_custom_command(OUTPUT "${shader_output}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${shader_output_dir}"
        COMMAND "${GLSLC_PROGRAM}" "${shader_source}" -o "${shader_output}"
        DEPENDS "${shader_source}" "${GLSLC_PROGRAM}"
        VERBATIM
    )
    list(APPEND shader_outputs "${shader_output}")
endforeach()

#链接glfw
target_link_libraries(${PROJECT_NAME} PRIVATE Vulkan::Vulkan glfw)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

#自动添加文件
add_custom_target(${PROJECT_NAME}_shaders
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${shader_outputs} "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
    DEPENDS ${shader_outputs}
    VERBATIM
)
add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}_shaders)
