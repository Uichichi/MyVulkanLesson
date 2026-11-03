#pragma once

#include "vkEngine/device.hpp"
#include "vulkan/vulkan.hpp"
#include <cstdint>
#include <string>
#include <vector>

namespace vkEG {
        struct PipelineConfigInfo{
            PipelineConfigInfo() = default;
            PipelineConfigInfo(const PipelineConfigInfo&) = delete;
            PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;
            
            vk::PipelineViewportStateCreateInfo viewportInfo;
            vk::PipelineRasterizationStateCreateInfo rasterizationInfo;
            vk::PipelineMultisampleStateCreateInfo multisampleInfo;
            vk::PipelineColorBlendAttachmentState colorBlendAttachment;
            vk::PipelineColorBlendStateCreateInfo colorBlendInfo;
            vk::PipelineDepthStencilStateCreateInfo depthStencilInfo;
            vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
            std::vector<vk::DynamicState> dynamicStateEnables;
            vk::PipelineDynamicStateCreateInfo dynamicStateInfo;
            vk::PipelineLayout pipelineLayout = nullptr;
            vk::RenderPass renderPass = nullptr;
            uint32_t subpass = 0;
        };
    class Pipeline{
        public:
            Pipeline(Device &device,const PipelineConfigInfo& configInfo,const std::string& vertFilepath,const std::string& fragFilepath);
            ~Pipeline();
            Pipeline(const Pipeline&) = delete;
            Pipeline& operator=(const Pipeline&) = delete;

            void bind(vk::CommandBuffer commandBuffer);
            static void defaultPipelineConfigInfo(PipelineConfigInfo& configInfo);
        private:
            static std::vector<char> readFile(const std::string& filepath);

            void createGraphicsPipeline(const PipelineConfigInfo& configInfo,const std::string& vertFilepath,const std::string& fragFilepath);
            void createShaderModle(const std::vector<char>& code,vk::ShaderModule& shaderModule);
            
            Device& device_;
            vk::Pipeline graphicsPipeline;
            vk::ShaderModule vertShaderModule;
            vk::ShaderModule fragShaderModule;
    };
}