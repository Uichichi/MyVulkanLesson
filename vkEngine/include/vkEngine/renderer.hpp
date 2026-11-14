#pragma once

#include <cassert>
#include <vector>
#include "memory"

#include "vkEngine/device.hpp"
#include "vkEngine/swapchain.hpp"
#include "vkEngine/window.hpp"



namespace vkEG {
    class Renderer{
        public:
            Renderer(Window& appWindow,Device& appDevice);
            ~Renderer();
            //删除拷贝构造和移动构造
            Renderer(const Renderer&) = delete;
            Renderer& operator=(const Renderer&) = delete;

            vk::RenderPass getSwapchainRenderPass() const {return swapchain_->getRenderPass();}
            bool isFrameInProgress() const{return isFrameStarted;};
            vk::CommandBuffer getCurrentCommandBuffer() const {
                assert(isFrameStarted && "Cannot get command buffer when frame is not in progress");
                return commandBuffers[currentFrameIndex];
            }

            vk::CommandBuffer beginFrame();
            void endFrame();
            void beginSwapchainRenderPass(vk::CommandBuffer commandBuffer);
            void endSwapchainRenderPass(vk::CommandBuffer commandBuffer);

            int getFrameIndex() const{
                assert(isFrameStarted && "Cannot get frame index when frame is not in progress");
                return currentFrameIndex;
            }

        private:
            void createCommandBuffers();
            void freeCommandBuffers();
            void recreateSwapchain();

            Window& window_;
            Device& device_;
            std::unique_ptr<Swapchain> swapchain_;
            std::vector<vk::CommandBuffer> commandBuffers;

            uint32_t currentImageIndex;
            int currentFrameIndex{0};
            bool isFrameStarted{false};
    };

}
