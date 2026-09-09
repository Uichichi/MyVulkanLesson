#pragma once

#include "window.hpp"
#include <GLFW/glfw3.h>
#include "vkEngine/pipeline.hpp"
#include "vkEngine/device.hpp"

namespace vkEG {
    class FirstApp{
        public:
            int WIDTH = 800;
            int HEIGHT = 600;

            void run();
        private:
            Window appWindow{WIDTH,HEIGHT,"HELLO VULKAN!"};
            Device appDevice{appWindow};
            Pipeline appPipeline{appDevice,Pipeline::defaultPipelineConfigInfo(WIDTH, HEIGHT),"simple_shader.vert.spv","simple_shader.frag.spv"};

    };

}