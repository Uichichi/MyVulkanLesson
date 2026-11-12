#pragma once

#include "vkEngine/model.hpp"
#include "vulkan/vulkan.hpp"
#include <GLFW/glfw3.h>

#include <vector>
#include "memory"

#include "vkEngine/pipeline.hpp"
#include "vkEngine/device.hpp"
#include "vkEngine/swapchain.hpp"
#include "vkEngine/window.hpp"
#include "vkEngine/window.hpp"
#include "vkEngine/object.hpp"

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
            void createPipelineLayout();
            void createPipeline();
            void createCommandBuffers();
            void freeCommandBuffers();
            void drawFrame();
            void recreateSwapchain();
            void recordCommandBuffer(int imageIndex);
            void renderObjects(vk::CommandBuffer commandBuffer);

            Window appWindow{WIDTH,HEIGHT,"HELLO VULKAN!"};
            Device appDevice{appWindow};
            std::unique_ptr<Swapchain> appSwapchain;
            //Pipeline appPipeline{appDevice,Pipeline::defaultPipelineConfigInfo(WIDTH, HEIGHT),"simple_shader.vert.spv","simple_shader.frag.spv"};
            std::unique_ptr<Pipeline> appPipeline;
            vk::PipelineLayout pipelineLayout;
            std::vector<vk::CommandBuffer> commandBuffers;
            std::vector<Object> objects;
    };

}