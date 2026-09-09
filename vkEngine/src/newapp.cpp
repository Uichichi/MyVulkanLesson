#include "vkEngine/newapp.hpp"
#include "GLFW/glfw3.h"

namespace vkEG {
    void FirstApp::run(){
        while (!appWindow.shouldClose()) {
            glfwPollEvents();
        }
    };


}