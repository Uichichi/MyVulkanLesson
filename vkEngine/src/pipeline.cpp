#include "vkEngine/pipeline.hpp"
#include "vkEngine/device.hpp"
#include "vulkan/vulkan.hpp"
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>

namespace vkEG{
    Pipeline::Pipeline(Device &device,const PipelineConfigInfo configInfo,const std::string& vertFilepath,const std::string& fragFilepath)
    : device_{device}{
        createGraphicsPipeline(configInfo,vertFilepath,fragFilepath);
    };
    Pipeline::~Pipeline(){
        device_.device().destroyShaderModule(vertShaderModule);
        device_.device().destroyShaderModule(fragShaderModule);
        device_.device().destroyPipeline(graphicsPipeline);
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
    void Pipeline::createGraphicsPipeline(const PipelineConfigInfo configInfo,const std::string& vertFilepath,const std::string& fragFilepath){
        auto vertCode = readFile(vertFilepath);
        auto fragCode = readFile(fragFilepath);

        createShaderModle(vertCode, vertShaderModule);
        createShaderModle(fragCode, fragShaderModule);

        if (configInfo.renderPass == nullptr || configInfo.pipelineLayout == nullptr) {
            return;
        }

        vk::PipelineShaderStageCreateInfo shaderStages[] = {
            vk::PipelineShaderStageCreateInfo{}
                .setStage(vk::ShaderStageFlagBits::eVertex)
                .setModule(vertShaderModule)
                .setPName("main"),
            vk::PipelineShaderStageCreateInfo{}
                .setStage(vk::ShaderStageFlagBits::eFragment)
                .setModule(fragShaderModule)
                .setPName("main")
        };

        vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.setVertexBindingDescriptionCount(0)
                       .setPVertexBindingDescriptions(nullptr)
                       .setVertexAttributeDescriptionCount(0)
                       .setPVertexAttributeDescriptions(nullptr);

        vk::GraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.setStageCount(2)
                    .setPStages(shaderStages)
                    .setPVertexInputState(&vertexInputInfo)
                    .setPInputAssemblyState(&configInfo.inputAssemblyInfo)
                    .setPViewportState(&configInfo.viewportInfo)
                    .setPRasterizationState(&configInfo.rasterizationInfo)
                    .setPMultisampleState(&configInfo.multisampleInfo)
                    .setPColorBlendState(&configInfo.colorBlendInfo)
                    .setPDepthStencilState(&configInfo.depthStencilInfo)
                    .setLayout(configInfo.pipelineLayout)
                    .setRenderPass(configInfo.renderPass)
                    .setSubpass(configInfo.subpass)
                    .setBasePipelineHandle(nullptr)
                    .setBasePipelineIndex(-1);

        auto result = device_.device().createGraphicsPipeline(nullptr, pipelineInfo);
        graphicsPipeline = result.value;
    };
    void Pipeline::createShaderModle(const std::vector<char>& code,vk::ShaderModule& shaderModule){
        vk::ShaderModuleCreateInfo createInfo;
        createInfo.setCodeSize(code.size())
                  .setPCode(reinterpret_cast<const uint32_t*>(code.data()));
        shaderModule = device_.device().createShaderModule(createInfo);
    };
    PipelineConfigInfo Pipeline::defaultPipelineConfigInfo(uint32_t width,uint32_t height){
            PipelineConfigInfo configInfo{};
            configInfo.inputAssemblyInfo.setTopology(vk::PrimitiveTopology::eTriangleList)
                                        .setPrimitiveRestartEnable(vk::False);

            configInfo.viewport.setX(0.0f)
                               .setY(0.0f)
                               .setWidth(static_cast<float>(width))
                               .setHeight(static_cast<float>(height))
                               .setMinDepth(0.0f)
                               .setMaxDepth(1.0f);

            configInfo.scissor.setOffset({0, 0})
                              .setExtent({width, height});

            configInfo.viewportInfo.setViewportCount(1)                               
                               .setPViewports(&configInfo.viewport)
                               .setScissorCount(1)
                               .setPScissors(&configInfo.scissor);

            configInfo.rasterizationInfo.setDepthClampEnable(vk::False)
                                        .setRasterizerDiscardEnable(vk::False)
                                        .setPolygonMode(vk::PolygonMode::eFill)
                                        .setLineWidth(1.0f)
                                        //丢弃模式
                                        .setCullMode(vk::CullModeFlagBits::eNone)
                                        .setFrontFace(vk::FrontFace::eClockwise)
                                        .setDepthBiasEnable(vk::False)
                                        //深度值暂时禁用
                                        .setDepthBiasClamp(0.0f)
                                        .setDepthBiasConstantFactor(0.0f)
                                        .setDepthBiasSlopeFactor(0.0f);
            
            configInfo.multisampleInfo.setSampleShadingEnable(vk::False)
                                      .setRasterizationSamples(vk::SampleCountFlagBits::e1)
                                      .setMinSampleShading(1.0f)
                                      .setPSampleMask(nullptr)
                                      .setAlphaToCoverageEnable(vk::False)
                                      .setAlphaToOneEnable(vk::False);

            configInfo.colorBlendAttachment.setColorWriteMask(vk::ColorComponentFlagBits::eR |
                                                                vk::ColorComponentFlagBits::eG |
                                                                vk::ColorComponentFlagBits::eB |
                                                                vk::ColorComponentFlagBits::eA)
                                           .setBlendEnable(vk::False)
                                           .setSrcColorBlendFactor(vk::BlendFactor::eOne)
                                           .setDstColorBlendFactor(vk::BlendFactor::eZero)
                                           .setColorBlendOp(vk::BlendOp::eAdd)
                                           .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
                                           .setDstAlphaBlendFactor(vk::BlendFactor::eZero)
                                           .setAlphaBlendOp(vk::BlendOp::eAdd);

            configInfo.colorBlendInfo.setLogicOpEnable(vk::False)
                                     .setLogicOp(vk::LogicOp::eCopy)
                                     .setAttachmentCount(1)
                                     .setPAttachments(&configInfo.colorBlendAttachment)
                                     .setBlendConstants({0.0f, 0.0f, 0.0f, 0.0f});

            configInfo.depthStencilInfo.setDepthTestEnable(vk::True)
                                       .setDepthWriteEnable(vk::True)
                                       .setDepthCompareOp(vk::CompareOp::eLess)
                                       .setDepthBoundsTestEnable(vk::False)
                                       .setMinDepthBounds(0.0f)
                                       .setMaxDepthBounds(1.0f)
                                       .setStencilTestEnable(vk::False)
                                       .setFront({})
                                       .setBack({});

            return configInfo;
        };
};