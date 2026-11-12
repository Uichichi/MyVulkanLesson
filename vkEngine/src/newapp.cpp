#include "vkEngine/newapp.hpp"
#include "glm/common.hpp"
#include "vkEngine/device.hpp"
#include "vkEngine/model.hpp"
#include "vkEngine/object.hpp"
#include "vkEngine/pipeline.hpp"

#include "vkEngine/swapchain.hpp"
#include "vulkan/vulkan.hpp"
#include "GLFW/glfw3.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_Z_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>


#include <array>
#include <cstdint>
#include <memory>
#include <vector>
#include "iostream"


namespace vkEG {
    struct SimplePushConstantData{
        glm::mat2 transform{1.f};
        glm::vec2 offset;
        //要求字节对齐
        alignas(16)
        glm::vec3 color;
    };
    NewApp::NewApp(){
        loadObjects();
        createPipelineLayout();
        //createPipeline();
        recreateSwapchain();
        createCommandBuffers();
    }

    NewApp::~NewApp(){
        appDevice.device().destroyPipelineLayout(pipelineLayout);
    }
    void NewApp::run(){
        while (!appWindow.shouldClose()) {
            glfwPollEvents();
            drawFrame();
        }
        appDevice.device().waitIdle();
    };

    void NewApp::createPipelineLayout(){
        vk::PushConstantRange pushConstantRange;
        pushConstantRange.setStageFlags(vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment)
                         .setOffset(0)
                         .setSize(sizeof(SimplePushConstantData));


        vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
        pipelineLayoutInfo.setSetLayoutCount(0)
                          .setPSetLayouts(nullptr)
                          .setPushConstantRangeCount(0)
                          .setPPushConstantRanges(&pushConstantRange);
        pipelineLayout = appDevice.device().createPipelineLayout(pipelineLayoutInfo);
    };
    void NewApp::createPipeline(){
        PipelineConfigInfo pipelineConfig{};
        Pipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = appSwapchain->getRenderPass();
        pipelineConfig.pipelineLayout = pipelineLayout;
        appPipeline = std::make_unique<Pipeline>(appDevice,pipelineConfig,"simple_shader.vert.spv","simple_shader.frag.spv");
    };
    void NewApp::createCommandBuffers(){
        commandBuffers.resize(appSwapchain->imageCount());

        vk::CommandBufferAllocateInfo allocInfo;

        allocInfo.setLevel(vk::CommandBufferLevel::ePrimary)
                 .setCommandPool(appDevice.getCommandPool())
                 .setCommandBufferCount(static_cast<uint32_t>(commandBuffers.size()));

        commandBuffers = appDevice.device().allocateCommandBuffers(allocInfo);
    }

    void NewApp::freeCommandBuffers(){
        appDevice.device().freeCommandBuffers(appDevice.getCommandPool(),static_cast<uint32_t>(commandBuffers.size()),commandBuffers.data());
        commandBuffers.clear();
    };

    // 录制清屏、视口和裁剪设置、管线绑定及模型绘制命令。
    void NewApp::recordCommandBuffer(int imageIndex){
        vk::CommandBufferBeginInfo beginInfo;
            commandBuffers[imageIndex].begin(beginInfo);

            vk::RenderPassBeginInfo renderPassInfo;
            renderPassInfo.setRenderPass(appSwapchain->getRenderPass())
                          .setFramebuffer(appSwapchain->getFramebuffer(imageIndex))
                          .setRenderArea({0,0})
                          //也可以vk::Rect2D{}
                          // .setOffset({0, 0})
                          // .setExtent(lveSwapChain.getSwapChainExtent())
                          .renderArea.setExtent(appSwapchain->getSwapchainExtent());
            std::array<vk::ClearValue, 2> clearValue{};
            clearValue[0].setColor({0.01f,0.01f,0.01f,1.0f});
            clearValue[1].setDepthStencil({1.0f,0});
            renderPassInfo.setClearValueCount(static_cast<uint32_t>(clearValue.size()))
                          .setClearValues(clearValue);
            commandBuffers[imageIndex].beginRenderPass(renderPassInfo,vk::SubpassContents::eInline);
            auto extent = appSwapchain->getSwapchainExtent();

            vk::Viewport viewport;

            viewport.setX(0.0f)
                    .setY(0.0f)
                    .setWidth(static_cast<float>(extent.width))
                    .setMinDepth(0.0f)
                    .setMaxDepth(1.0f)
                    .setHeight(static_cast<float>(extent.height));

            vk::Rect2D scissor;
            scissor.setOffset({0, 0})
                   .setExtent(extent);

            commandBuffers[imageIndex].setViewport(0, viewport);
            commandBuffers[imageIndex].setScissor(0, scissor);  
            
            renderObjects(commandBuffers[imageIndex]);

            commandBuffers[imageIndex].endRenderPass();
            commandBuffers[imageIndex].end();
    }

    void NewApp::renderObjects(vk::CommandBuffer commandBuffer){
        appPipeline->bind(commandBuffer);
        
        for (auto& obj: objects) {
            obj.transform2d.rotation = glm::mod(obj.transform2d.rotation + 0.01f,glm::two_pi<float>());

            SimplePushConstantData push{};
            push.offset = obj.transform2d.transLation;
            //颜色从深蓝到浅蓝
            push.color = obj.color;
            push.transform = obj.transform2d.mat2();
            //推送常量通常用于存储变换矩阵
            commandBuffer.pushConstants(pipelineLayout,vk::ShaderStageFlagBits::eVertex|vk::ShaderStageFlagBits::eFragment,0,sizeof(SimplePushConstantData),&push);
            
            obj.model->bind(commandBuffer);
            obj.model->draw(commandBuffer);
        }
    };

    // 等待窗口尺寸有效且设备空闲后重建交换链，按需重分配命令缓冲并重建管线。
    void NewApp::recreateSwapchain(){
        auto extent = appWindow.getExtent();
        while (extent.width == 0 || extent.height == 0) {
            extent = appWindow.getExtent();
            glfwWaitEvents();
        }
        appDevice.device().waitIdle();
        if (appSwapchain == nullptr) {
            appSwapchain = std::make_unique<Swapchain>(appDevice, extent);
        } else {
            appSwapchain = std::make_unique<Swapchain>(appDevice, extent, std::move(appSwapchain));
        }
        if (!commandBuffers.empty() && commandBuffers.size() != appSwapchain->imageCount()) {
            freeCommandBuffers();
            createCommandBuffers();
        }
        //如果渲染管线兼容，则无需创建新管线
        createPipeline();
    }
    // 获取交换链图像、录制并提交绘制，在交换链失效或窗口缩放时重建。
    void NewApp::drawFrame(){
        uint32_t imageIndex;
        auto result = appSwapchain->acquireNextImage(&imageIndex);
        if (result == vk::Result::eErrorOutOfDateKHR) {
            recreateSwapchain();
            return;
        }
        if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
            std::cout << "acquire swapchain image failed" << std::endl;
            return;
        }
        recordCommandBuffer(imageIndex);
        result = appSwapchain->submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);
        if (result == vk::Result::eErrorOutOfDateKHR ||result == vk::Result::eSuboptimalKHR || appWindow.wasWindowResized()) {
            appWindow.resetWindowResizedFlag();
            recreateSwapchain();
            return;
        }
        if (result != vk::Result::eSuccess) {
            std::cout << "present swapchain image failed" << std::endl;
        }
    }
    void NewApp::loadObjects(){
        std::vector<Model::Vertex> vertices{
            {{0.0f,-0.5f},{1.0f,0.0f,0.0f}},
            {{0.5f,0.5f},{0.0f,1.0f,0.0f}},
            {{-0.5f,0.5f},{0.0f,0.0f,1.0f}}
        };
        auto model = std::make_shared<Model>(appDevice,vertices);

        auto triangle = Object::createObject();
        triangle.model = model;
        triangle.color = {.1f,.8f,.1f};
        triangle.transform2d.transLation.x = .2f;
        triangle.transform2d.scale  ={.2f,.5f};
        //vulkan的y轴和常规数学相反，y在x轴上是负数
        triangle.transform2d.rotation = .25f * glm::two_pi<float>();

        objects.push_back(std::move(triangle));
    };

}