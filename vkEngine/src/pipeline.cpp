#include "vkEngine/pipeline.hpp"
#include "vkEngine/device.hpp"
#include "vulkan/vulkan.hpp"
#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>

namespace vkEG{
    Pipeline::Pipeline(Device &device,const PipelineConfigInfo configInfo,const std::string& vertFilepath,const std::string& fragFilepath)
    : device_{device}{
        createGraphicsPipeline(configInfo,vertFilepath,fragFilepath);
    };
    std::vector<char> Pipeline::readFile(const std::string& filepath){
        std::ifstream file{filepath,std::ios::ate|std::ios::binary};

        if(!file.is_open()){
            std::cout<<"open file failed"<<filepath<<std::endl;
        }
        size_t fileSize =  static_cast<size_t>(file.tellg());
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();
        return buffer;
    };
    void Pipeline::createGraphicsPipeline(const PipelineConfigInfo ConfigInfo,const std::string& vertFilepath,const std::string& fragFilepath){
        auto vertCode = readFile(vertFilepath);
        auto fragCode = readFile(fragFilepath);
    };
    void Pipeline::createShaderModle(const std::vector<char>& code,vk::ShaderModule& shaderModule){
        vk::ShaderModuleCreateInfo createInfo;
        createInfo.setCodeSize(code.size())
                  .setPCode(reinterpret_cast<const uint32_t*>(code.data()));
        shaderModule = device_.device().createShaderModule(createInfo);
    };
    PipelineConfigInfo Pipeline::defaultPipelineConfigInfo(uint32_t width,uint32_t height){
            PipelineConfigInfo configInfo{};
            
            return configInfo;
        };
};