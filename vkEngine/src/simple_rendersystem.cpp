#include "vkEngine/simple_rendersystem.hpp"
#include "vkEngine/device.hpp"
#include "vulkan/vulkan.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_Z_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <memory>
#include <vector>



namespace vkEG {
    struct SimplePushConstantData{
        glm::mat2 transform{1.f};
        glm::vec2 offset;
        //要求字节对齐
        alignas(16)
        glm::vec3 color;
    };
    SimpleRendersystem::SimpleRendersystem(Device& device,vk::RenderPass renderPass):device_(device) {
        createPipelineLayout();
        createPipeline(renderPass);
    }

    SimpleRendersystem::~SimpleRendersystem(){
        device_.device().destroyPipelineLayout(pipelineLayout_);
    }


    void SimpleRendersystem::createPipelineLayout(){
        vk::PushConstantRange pushConstantRange;
        pushConstantRange.setStageFlags(vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment)
                         .setOffset(0)
                         .setSize(sizeof(SimplePushConstantData));


        vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
        pipelineLayoutInfo.setSetLayoutCount(0)
                          .setPSetLayouts(nullptr)
                          .setPushConstantRangeCount(1)
                          .setPPushConstantRanges(&pushConstantRange);
        pipelineLayout_ = device_.device().createPipelineLayout(pipelineLayoutInfo);
    };
    void SimpleRendersystem::createPipeline(vk::RenderPass renderPass){
        PipelineConfigInfo pipelineConfig{};
        Pipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = pipelineLayout_;
        pipeline_ = std::make_unique<Pipeline>(device_,pipelineConfig,"simple_shader.vert.spv","simple_shader.frag.spv");
    };

    void SimpleRendersystem::renderObjects(vk::CommandBuffer commandBuffer,std::vector<Object>& objects){
        pipeline_->bind(commandBuffer);

        for (auto& obj: objects) {
            obj.transform2d.rotation = glm::mod(obj.transform2d.rotation + 0.01f,glm::two_pi<float>());

            SimplePushConstantData push{};
            push.offset = obj.transform2d.transLation;
            //颜色从深蓝到浅蓝
            push.color = obj.color;
            push.transform = obj.transform2d.mat2();
            //推送常量通常用于存储变换矩阵
            commandBuffer.pushConstants(pipelineLayout_,vk::ShaderStageFlagBits::eVertex|vk::ShaderStageFlagBits::eFragment,0,sizeof(SimplePushConstantData),&push);

            obj.model->bind(commandBuffer);
            obj.model->draw(commandBuffer);
        }
    };

}
