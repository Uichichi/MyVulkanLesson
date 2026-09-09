#pragma once

#include "vkEngine/device.hpp"
#include "vulkan/vulkan.hpp"
#include <cstdint>
#include <string>
#include <vector>

namespace vkEG {
        struct PipelineConfigInfo{};
    class Pipeline{
        public:
            Pipeline(Device &device,const PipelineConfigInfo configInfo,const std::string& vertFilepath,const std::string& fragFilepath);
            ~Pipeline(){};
            Pipeline(const Pipeline&) = delete;
            void operator=(const Pipeline&) = delete;

            static PipelineConfigInfo defaultPipelineConfigInfo(uint32_t width,uint32_t height);
        private:
            static std::vector<char> readFile(const std::string& filepath);

            void createGraphicsPipeline(const PipelineConfigInfo configInfo,const std::string& vertFilepath,const std::string& fragFilepath);
            void createShaderModle(const std::vector<char>& code,vk::ShaderModule& shaderModule);
            
            Device& device_;
            vk::Pipeline graphicsPipeline;
            vk::ShaderModule vertShaderModule;
            vk::ShaderModule fragShaderModule;
    };
}