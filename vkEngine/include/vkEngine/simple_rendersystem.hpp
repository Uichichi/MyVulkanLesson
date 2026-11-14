#pragma once

#include "vulkan/vulkan.hpp"
#include <GLFW/glfw3.h>

#include <vector>
#include "memory"

#include "vkEngine/pipeline.hpp"
#include "vkEngine/object.hpp"
#include "vkEngine/device.hpp"


namespace vkEG {
    class SimpleRendersystem{
        public:
            SimpleRendersystem(Device& device,vk::RenderPass renderPass);
            ~SimpleRendersystem();
            //删除拷贝构造和移动构造
            SimpleRendersystem(const SimpleRendersystem&) = delete;
            SimpleRendersystem& operator=(const SimpleRendersystem&) = delete;
            void renderObjects(vk::CommandBuffer commandBuffer,std::vector<Object>& objects);
        private:
            void createPipelineLayout();
            void createPipeline(vk::RenderPass renderPass);


            Device& device_;

            //Pipeline appPipeline{appDevice,Pipeline::defaultPipelineConfigInfo(WIDTH, HEIGHT),"simple_shader.vert.spv","simple_shader.frag.spv"};
            std::unique_ptr<Pipeline> pipeline_;
            vk::PipelineLayout pipelineLayout_;
    };

}
