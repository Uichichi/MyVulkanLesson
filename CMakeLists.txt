cmake_minimum_required(VERSION 4.0)

project(VulkanWorkspace)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


# ==========================
# 自动添加第三方库include
# ==========================

set(LIBRARY_DIR ${CMAKE_SOURCE_DIR}/library)


file(GLOB LIBRARIES
    ${LIBRARY_DIR}/*
)


foreach(LIBRARY ${LIBRARIES})

    if(IS_DIRECTORY ${LIBRARY})

        message(STATUS "Found library: ${LIBRARY}")
         # 自动扫描库根目录
        if(EXISTS "${LIBRARY}")
            include_directories("${LIBRARY}")
            message(STATUS "Add include: ${LIBRARY}")
        endif()
        # 自动扫描 include 目录
        if(IS_DIRECTORY "${LIBRARY}/include")
            include_directories("${LIBRARY}/include")
            message(STATUS "Add include: ${LIBRARY}/include")
        endif()

    endif()

endforeach()

#添加Vulkan库
find_package(Vulkan REQUIRED)

link_libraries(Vulkan::Vulkan)
# ==========================
# 添加项目
# ==========================
#file(GLOB PROJECTS
#    RELATIVE ${CMAKE_SOURCE_DIR}
#    "*"
#)

#foreach(PROJECT_DIR ${PROJECTS})
#   if(
#        IS_DIRECTORY ${PROJECT_DIR}
#        AND EXISTS ${PROJECT_DIR}/CMakeLists.txt
#    )
#        add_subdirectory(${PROJECT_DIR})
#
#    endif()
#
#endforeach()

file(GLOB CHILD_PROJECTS
    CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/*"
)
foreach(PROJECT_DIR ${CHILD_PROJECTS})
        if(
            IS_DIRECTORY ${PROJECT_DIR} 
            AND EXISTS ${PROJECT_DIR}/CMakeLists.txt
        )
        add_subdirectory("${PROJECT_DIR}")
    else()
    endif()
endforeach()

#加入glfw编译
add_subdirectory(
    ${CMAKE_SOURCE_DIR}/library/glfw-3.5.1
)