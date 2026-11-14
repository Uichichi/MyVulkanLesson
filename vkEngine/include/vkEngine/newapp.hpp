#pragma once

#include "vulkan/vulkan.hpp"
#include <GLFW/glfw3.h>

#include <vector>
#include "memory"


#include "vkEngine/device.hpp"
#include "vkEngine/window.hpp"
#include "vkEngine/object.hpp"
#include "vkEngine/renderer.hpp"

namespace vkEG {
    class NewApp{
        public:
            int WIDTH = 800;
            int HEIGHT = 600;

            NewApp();
            ~NewApp();
            //删除拷贝构造和移动构造
            NewApp(const NewApp&) = delete;
            NewApp& operator=(const NewApp&) = delete;

            void run();

        private:
            void loadObjects();

            Window appWindow{WIDTH,HEIGHT,"HELLO VULKAN!"};
            Device appDevice{appWindow};
            Renderer appRenderer{appWindow,appDevice};

            std::vector<Object> objects;
    };

}