#include "vkEngine/newapp.hpp"
#include "vkEngine/device.hpp"
#include "vkEngine/pipeline.hpp"

#include "vulkan/vulkan.hpp"
#include "GLFW/glfw3.h"

#include <array>
#include <cstdint>
#include <memory>
#include "iostream"


namespace vkEG {
    FirstApp::FirstApp(){
        createPipelineLayout();
        createPipeline();
        createCommandBuffers();
    }

    FirstApp::~FirstApp(){
        appDevice.device().destroyPipelineLayout(pipelineLayout);
    }
    void FirstApp::run(){
        while (!appWindow.shouldClose()) {
            glfwPollEvents();
            drawFrame();
        }
        appDevice.device().waitIdle();
    };

    void FirstApp::createPipelineLayout(){
        vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
        pipelineLayoutInfo.setSetLayoutCount(0)
                          .setPSetLayouts(nullptr)
                          .setPushConstantRangeCount(0)
                          .setPPushConstantRanges(nullptr);
        pipelineLayout = appDevice.device().createPipelineLayout(pipelineLayoutInfo);
    };
    void FirstApp::createPipeline(){
        auto pipelineConfig = Pipeline::defaultPipelineConfigInfo(appSwapchain.width(),appSwapchain.height());
        pipelineConfig.renderPass = appSwapchain.getRenderPass();
        pipelineConfig.pipelineLayout = pipelineLayout;
        appPipeline = std::make_unique<Pipeline>(appDevice,pipelineConfig,"simple_shader.vert.spv","simple_shader.frag.spv");
    };
    void FirstApp::createCommandBuffers(){
        commandBuffer.resize(appSwapchain.imageCount());

        vk::CommandBufferAllocateInfo allocInfo;

        allocInfo.setLevel(vk::CommandBufferLevel::ePrimary)
                 .setCommandPool(appDevice.getCommandPool())
                 .setCommandBufferCount(static_cast<uint32_t>(commandBuffer.size()));

        commandBuffer = appDevice.device().allocateCommandBuffers(allocInfo);

        for (int i = 0; i < commandBuffer.size(); i++) {
            vk::CommandBufferBeginInfo beginInfo;
            commandBuffer[i].begin(beginInfo);

            vk::RenderPassBeginInfo renderPassInfo;
            renderPassInfo.setRenderPass(appSwapchain.getRenderPass())
                          .setFramebuffer(appSwapchain.getFramebuffer(i))
                          .setRenderArea({0,0})
                          //也可以vk::Rect2D{}
                          // .setOffset({0, 0})
                          // .setExtent(lveSwapChain.getSwapChainExtent())
                          .renderArea.setExtent(appSwapchain.getSwapchainExtent());
            std::array<vk::ClearValue, 2> clearValue{};
            clearValue[0].setColor({0.1f,0.1f,0.1f,1.0f});
            clearValue[1].setDepthStencil({1.0f,0});
            renderPassInfo.setClearValueCount(static_cast<uint32_t>(clearValue.size()))
                          .setClearValues(clearValue);
            commandBuffer[i].beginRenderPass(renderPassInfo,vk::SubpassContents::eInline);
            
            appPipeline->bind(commandBuffer[i]);
            commandBuffer[i].draw(3, 1, 0, 0);

            commandBuffer[i].endRenderPass();
            commandBuffer[i].end();
        }

    }
    void FirstApp::drawFrame(){
        uint32_t imageIndex;
        auto result = appSwapchain.acquireNextImage(&imageIndex);
        if (result != vk::Result::eSuccess) {
            std::cout << "acquire swapchain image failed" << std::endl;
        }
        result = appSwapchain.submitCommandBuffers(&commandBuffer[imageIndex], &imageIndex);
        if (result != vk::Result::eSuccess) {
            std::cout << "present swapchain image failed" << std::endl;
        }
    }
}