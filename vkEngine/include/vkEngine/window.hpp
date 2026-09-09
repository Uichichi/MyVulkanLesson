#pragma once
#include "vulkan/vulkan.hpp"
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include <string>
namespace vkEG {
    class Window{
        public:
            Window(int w,int h,std::string name);
            ~Window();
            //删除拷贝构造和移动构造
            Window(const Window&) = delete;
            Window& operator=(const Window&) = delete;
            bool shouldClose();
            //为防止改太多，这里surface就保留指针类型
            void createWindowSurface(vk::Instance instance,vk::SurfaceKHR *surface);
            
        private:
            void initWindow();
            int width_;
            int height_;

            std::string windowName_;
            GLFWwindow *window_;
    };
}