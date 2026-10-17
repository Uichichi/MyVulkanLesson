#pragma once

#include "vkEngine/device.hpp"

#include <cstdint>
#include <vector>

namespace vkEG {

class Swapchain {
public:
    static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

    Swapchain(Device& device, vk::Extent2D windowExtent);
    ~Swapchain();

    Swapchain(const Swapchain&) = delete;
    Swapchain& operator=(const Swapchain&) = delete;
    Swapchain(Swapchain&&) = delete;
    Swapchain& operator=(Swapchain&&) = delete;

    vk::Framebuffer getFramebuffer(uint32_t index) const { return swapchainFramebuffers_[index]; }
    vk::RenderPass getRenderPass() const { return renderPass_; }
    vk::ImageView getImageView(uint32_t index) const { return swapchainImageViews_[index]; }
    size_t imageCount() const { return swapchainImages_.size(); }
    vk::Format getSwapchainImageFormat() const { return swapchainImageFormat_; }
    vk::Extent2D getSwapchainExtent() const { return swapchainExtent_; }
    uint32_t width() const { return swapchainExtent_.width; }
    uint32_t height() const { return swapchainExtent_.height; }

    float extentAspectRatio() const {
        return static_cast<float>(swapchainExtent_.width) / static_cast<float>(swapchainExtent_.height);
    }

    vk::Format findDepthFormat();
    vk::Result acquireNextImage(uint32_t* imageIndex);
    vk::Result submitCommandBuffers(const vk::CommandBuffer* buffers, uint32_t* imageIndex);

private:
    void createSwapchain();
    void createImageViews();
    void createDepthResources();
    void createRenderPass();
    void createFramebuffers();
    void createSyncObjects();

    vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
    vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);
    vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities);

    vk::Format swapchainImageFormat_{};
    vk::Extent2D swapchainExtent_{};

    std::vector<vk::Framebuffer> swapchainFramebuffers_;
    vk::RenderPass renderPass_{};

    std::vector<vk::Image> depthImages_;
    std::vector<vk::DeviceMemory> depthImageMemories_;
    std::vector<vk::ImageView> depthImageViews_;
    std::vector<vk::Image> swapchainImages_;
    std::vector<vk::ImageView> swapchainImageViews_;

    Device& device_;
    vk::Extent2D windowExtent_;
    vk::SwapchainKHR swapchain_{};

    std::vector<vk::Semaphore> imageAvailableSemaphores_;
    std::vector<vk::Semaphore> renderFinishedSemaphores_;
    std::vector<vk::Fence> inFlightFences_;
    std::vector<vk::Fence> imagesInFlight_;
    size_t currentFrame_ = 0;
};

} // namespace vkEG
