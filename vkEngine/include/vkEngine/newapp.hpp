#pragma once

#include "vulkan/vulkan.hpp"
#include "window.hpp"
#include <GLFW/glfw3.h>
#include <vector>
#include "vkEngine/pipeline.hpp"
#include "vkEngine/device.hpp"
#include "vkEngine/swapchain.hpp"
#include "memory"

namespace vkEG {
    class FirstApp{
        public:
            int WIDTH = 800;
            int HEIGHT = 600;

            FirstApp();
            ~FirstApp();
            //删除拷贝构造和移动构造
            FirstApp(const FirstApp&) = delete;
            FirstApp& operator=(const FirstApp&) = delete;

            void run();

        private:
            void createPipelineLayout();
            void createPipeline();
            void createCommandBuffers();
            void drawFrame();
            Window appWindow{WIDTH,HEIGHT,"HELLO VULKAN!"};
            Device appDevice{appWindow};
            Swapchain appSwapchain{appDevice,appWindow.getExtent()};
            //Pipeline appPipeline{appDevice,Pipeline::defaultPipelineConfigInfo(WIDTH, HEIGHT),"simple_shader.vert.spv","simple_shader.frag.spv"};
            std::unique_ptr<Pipeline> appPipeline;
            vk::PipelineLayout pipelineLayout;
            std::vector<vk::CommandBuffer> commandBuffer;
    };

}