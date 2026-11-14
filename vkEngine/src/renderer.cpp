#include "vkEngine/renderer.hpp"


#include "GLFW/glfw3.h"

#include <array>
#include <cassert>
#include <iostream>
#include <memory>
#include "vkEngine/swapchain.hpp"


namespace vkEG {
    Renderer::Renderer(Window& appWindow,Device& appDevice):window_(appWindow),device_(appDevice){
        recreateSwapchain();
        createCommandBuffers();
    }

    Renderer::~Renderer(){
        freeCommandBuffers();
    }

    void Renderer::createCommandBuffers(){
        commandBuffers.resize(Swapchain::MAX_FRAMES_IN_FLIGHT);

        vk::CommandBufferAllocateInfo allocInfo;

        allocInfo.setLevel(vk::CommandBufferLevel::ePrimary)
                 .setCommandPool(device_.getCommandPool())
                 .setCommandBufferCount(static_cast<uint32_t>(commandBuffers.size()));

        commandBuffers = device_.device().allocateCommandBuffers(allocInfo);
    }

    void Renderer::freeCommandBuffers(){
        device_.device().freeCommandBuffers(device_.getCommandPool(),static_cast<uint32_t>(commandBuffers.size()),commandBuffers.data());
        commandBuffers.clear();
    };

    // 等待窗口尺寸有效且设备空闲后重建交换链，检查渲染通道兼容性。
    void Renderer::recreateSwapchain(){
        auto extent = window_.getExtent();
        while (extent.width == 0 || extent.height == 0) {
            extent = window_.getExtent();
            glfwWaitEvents();
        }
        device_.device().waitIdle();
        if (swapchain_ == nullptr) {
            swapchain_ = std::make_unique<Swapchain>(device_, extent);
        } else {
            std::shared_ptr<Swapchain> oldSwapchain = std::move(swapchain_);
            swapchain_ = std::make_unique<Swapchain>(device_, extent, oldSwapchain);
            if (!oldSwapchain->compareSwapchainFormat(*swapchain_)) {
                std::cout << "Swapchain image or depth format has changed" << std::endl;
            }
        }
    }

    vk::CommandBuffer Renderer::beginFrame(){
        assert(!isFrameStarted && "Cannot begin a frame while a frame is in progress");
        auto result = swapchain_->acquireNextImage(&currentImageIndex);
        if (result == vk::Result::eErrorOutOfDateKHR) {
            recreateSwapchain();
            return nullptr;
        }
        if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
            std::cout << "Failed to acquire swapchain image" << std::endl;
            return nullptr;
        }
        isFrameStarted = true;
        auto commandBuffer = getCurrentCommandBuffer();
        vk::CommandBufferBeginInfo beginInfo;
        commandBuffer.begin(beginInfo);
        return commandBuffer;
    };
    void Renderer::endFrame(){
        assert(isFrameStarted && "Cannot end a frame when no frame is in progress");
        auto commandBuffer = getCurrentCommandBuffer();
        commandBuffer.end();
        auto result = swapchain_->submitCommandBuffers(&commandBuffer, &currentImageIndex);
        if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || window_.wasWindowResized()) {
            window_.resetWindowResizedFlag();
            recreateSwapchain();
        } else if (result != vk::Result::eSuccess) {
            std::cout << "Failed to present swapchain image" << std::endl;
        }
        isFrameStarted = false;
        currentFrameIndex = (currentFrameIndex + 1) % Swapchain::MAX_FRAMES_IN_FLIGHT;
    };
    void Renderer::beginSwapchainRenderPass(vk::CommandBuffer commandBuffer){
        assert(isFrameStarted && "Cannot begin a render pass outside a frame");
        assert(commandBuffer == getCurrentCommandBuffer() && "begin renderpass on commandBuffer from different frame");
        vk::RenderPassBeginInfo renderPassInfo;
        renderPassInfo.setRenderPass(swapchain_->getRenderPass())
                      .setFramebuffer(swapchain_->getFramebuffer(currentImageIndex))
                      .setRenderArea({0,0})
                      //也可以vk::Rect2D{}
                      // .setOffset({0, 0})
                      // .setExtent(lveSwapChain.getSwapChainExtent())
                      .renderArea.setExtent(swapchain_->getSwapchainExtent());
        std::array<vk::ClearValue, 2> clearValue{};
        clearValue[0].setColor({0.01f,0.01f,0.01f,1.0f});
        clearValue[1].setDepthStencil({1.0f,0});
        renderPassInfo.setClearValueCount(static_cast<uint32_t>(clearValue.size()))
                      .setClearValues(clearValue);
        commandBuffer.beginRenderPass(renderPassInfo,vk::SubpassContents::eInline);
        auto extent = swapchain_->getSwapchainExtent();

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

        commandBuffer.setViewport(0, viewport);
        commandBuffer.setScissor(0, scissor);
    };
    void Renderer::endSwapchainRenderPass(vk::CommandBuffer commandBuffer){
        assert(isFrameStarted && "Cannot end a render pass outside a frame");
        assert(commandBuffer == getCurrentCommandBuffer() && "end renderpass on commandBuffer from different frame");
        commandBuffer.endRenderPass();
    };
}
