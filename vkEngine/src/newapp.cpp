#include "vkEngine/newapp.hpp"
#include "vkEngine/device.hpp"
#include "vkEngine/model.hpp"
#include "vkEngine/pipeline.hpp"

#include "vkEngine/swapchain.hpp"
#include "vulkan/vulkan.hpp"
#include "GLFW/glfw3.h"

#include <array>
#include <cstdint>
#include <memory>
#include <vector>
#include "iostream"


namespace vkEG {
    FirstApp::FirstApp(){
        loadModels();
        createPipelineLayout();
        //createPipeline();
        recreateSwapchain();
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
        PipelineConfigInfo pipelineConfig{};
        Pipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = appSwapchain->getRenderPass();
        pipelineConfig.pipelineLayout = pipelineLayout;
        appPipeline = std::make_unique<Pipeline>(appDevice,pipelineConfig,"simple_shader.vert.spv","simple_shader.frag.spv");
    };
    void FirstApp::createCommandBuffers(){
        commandBuffers.resize(appSwapchain->imageCount());

        vk::CommandBufferAllocateInfo allocInfo;

        allocInfo.setLevel(vk::CommandBufferLevel::ePrimary)
                 .setCommandPool(appDevice.getCommandPool())
                 .setCommandBufferCount(static_cast<uint32_t>(commandBuffers.size()));

        commandBuffers = appDevice.device().allocateCommandBuffers(allocInfo);
    }

    void FirstApp::freeCommandBuffers(){
        appDevice.device().freeCommandBuffers(appDevice.getCommandPool(),static_cast<uint32_t>(commandBuffers.size()),commandBuffers.data());
        commandBuffers.clear();
    };

    void FirstApp::recordCommandBuffer(int imageIndex){
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
            clearValue[0].setColor({0.1f,0.1f,0.1f,1.0f});
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
                    
            appPipeline->bind(commandBuffers[imageIndex]);
            //删除
            // commandBuffer[i].draw(3, 1, 0, 0);

            model->bind(commandBuffers[imageIndex]);
            model->draw(commandBuffers[imageIndex]);

            commandBuffers[imageIndex].endRenderPass();
            commandBuffers[imageIndex].end();
    }
    void FirstApp::recreateSwapchain(){
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
    void FirstApp::drawFrame(){
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
    void FirstApp::loadModels(){
        std::vector<Model::Vertex> vertices{
            {{0.0f,-0.5f},{1.0f,0.0f,0.0f}},
            {{0.5f,0.5f},{0.0f,1.0f,0.0f}},
            {{-0.5f,0.5f},{0.0f,0.0f,1.0f}}
        };
        model = std::make_unique<Model>(appDevice,vertices);
    };
}