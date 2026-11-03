#include "vkEngine/window.hpp"
#include "GLFW/glfw3.h"
#include <iostream>
#include <string>

namespace vkEG {
    Window::Window(int w,int h,std::string name): width_{w},height_{h},windowName_{name}{
        initWindow();
    };
    Window::~Window(){
        glfwDestroyWindow(window_);
    };
    void Window::initWindow(){
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        window_ = glfwCreateWindow(width_, height_, windowName_.c_str(), nullptr, nullptr);
        glfwSetWindowUserPointer(window_, this);
        glfwSetFramebufferSizeCallback(window_, framebufferResizeCallback);
    };
    bool Window::shouldClose(){
        return glfwWindowShouldClose(window_);
    };
    //记得将surface变成cpp风格存储
    void Window::createWindowSurface(vk::Instance instance,vk::SurfaceKHR *surface){
        //使用c风格的中间变量
        VkSurfaceKHR tempSurface;
        if (glfwCreateWindowSurface(instance, window_, nullptr, &tempSurface) != VK_SUCCESS) {
            std::cout<<"create surface failed/n"<<std::endl;
        }
        *surface = tempSurface;
    };
    void Window::framebufferResizeCallback(GLFWwindow *window,int width,int height){
        auto window_ = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));

        window_->framebufferResized = true;
        window_->width_ = width;
        window_->height_ =  height;
    };
}