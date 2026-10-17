#include "vkEngine/swapchain.hpp"

#include <array>
#include <algorithm>
#include <iostream>
#include <limits>

namespace vkEG {

Swapchain::Swapchain(Device& device, vk::Extent2D windowExtent)
    : device_{device}, windowExtent_{windowExtent} {
    createSwapchain();
    if (!swapchain_) return;
    createImageViews();
    createRenderPass();
    createDepthResources();
    createFramebuffers();
    createSyncObjects();
}

Swapchain::~Swapchain() {
    const vk::Device logicalDevice = device_.device();

    for (auto imageView : swapchainImageViews_) {
        if (imageView) logicalDevice.destroyImageView(imageView);
    }
    if (swapchain_) logicalDevice.destroySwapchainKHR(swapchain_);

    for (size_t i = 0; i < depthImages_.size(); ++i) {
        if (i < depthImageViews_.size() && depthImageViews_[i]) logicalDevice.destroyImageView(depthImageViews_[i]);
        if (depthImages_[i]) logicalDevice.destroyImage(depthImages_[i]);
        if (i < depthImageMemories_.size() && depthImageMemories_[i]) logicalDevice.freeMemory(depthImageMemories_[i]);
    }

    for (auto framebuffer : swapchainFramebuffers_) {
        if (framebuffer) logicalDevice.destroyFramebuffer(framebuffer);
    }
    if (renderPass_) logicalDevice.destroyRenderPass(renderPass_);

    for (auto semaphore : renderFinishedSemaphores_) {
        if (semaphore) logicalDevice.destroySemaphore(semaphore);
    }
    for (auto semaphore : imageAvailableSemaphores_) {
        if (semaphore) logicalDevice.destroySemaphore(semaphore);
    }
    for (auto fence : inFlightFences_) {
        if (fence) logicalDevice.destroyFence(fence);
    }
}

vk::Result Swapchain::acquireNextImage(uint32_t* imageIndex) {
    if (!imageIndex || currentFrame_ >= inFlightFences_.size()) return vk::Result::eErrorInitializationFailed;

    vk::Result result = device_.device().waitForFences(1, &inFlightFences_[currentFrame_], VK_TRUE, std::numeric_limits<uint64_t>::max());
    if (result != vk::Result::eSuccess) {
        std::cout << "wait for frame fence failed" << std::endl;
        return result;
    }

    return device_.device().acquireNextImageKHR(
        swapchain_, std::numeric_limits<uint64_t>::max(), imageAvailableSemaphores_[currentFrame_], vk::Fence{}, imageIndex);
}

vk::Result Swapchain::submitCommandBuffers(const vk::CommandBuffer* buffers, uint32_t* imageIndex) {
    if (!buffers || !imageIndex || *imageIndex >= imagesInFlight_.size()) return vk::Result::eErrorInitializationFailed;

    if (imagesInFlight_[*imageIndex]) {
        vk::Result waitResult = device_.device().waitForFences(1, &imagesInFlight_[*imageIndex], VK_TRUE, UINT64_MAX);
        if (waitResult != vk::Result::eSuccess) {
            std::cout << "wait for image fence failed" << std::endl;
            return waitResult;
        }
    }
    imagesInFlight_[*imageIndex] = inFlightFences_[currentFrame_];

    const vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
    const vk::Semaphore waitSemaphores[] = {imageAvailableSemaphores_[currentFrame_]};
    const vk::Semaphore signalSemaphores[] = {renderFinishedSemaphores_[currentFrame_]};

    vk::SubmitInfo submitInfo;
    submitInfo.setWaitSemaphoreCount(1)
        .setPWaitSemaphores(waitSemaphores)
        .setPWaitDstStageMask(waitStages)
        .setCommandBufferCount(1)
        .setPCommandBuffers(buffers)
        .setSignalSemaphoreCount(1)
        .setPSignalSemaphores(signalSemaphores);

    vk::Result result = device_.graphicsQueue().submit(1, &submitInfo, inFlightFences_[currentFrame_]);
    if (result != vk::Result::eSuccess) {
        std::cout << "submit draw command buffer failed" << std::endl;
        return result;
    }

    vk::PresentInfoKHR presentInfo;
    presentInfo.setWaitSemaphoreCount(1)
        .setPWaitSemaphores(signalSemaphores)
        .setSwapchainCount(1)
        .setPSwapchains(&swapchain_)
        .setPImageIndices(imageIndex);

    result = device_.presentQueue().presentKHR(&presentInfo);
    currentFrame_ = (currentFrame_ + 1) % MAX_FRAMES_IN_FLIGHT;
    return result;
}

void Swapchain::createSwapchain() {
    const SwapChainSupportDetails support = device_.getSwapChainSupport();
    if (support.formats.empty() || support.presentModes.empty()) {
        std::cout << "swapchain support has no formats or present modes" << std::endl;
        return;
    }

    const vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(support.formats);
    const vk::PresentModeKHR presentMode = chooseSwapPresentMode(support.presentModes);
    const vk::Extent2D extent = chooseSwapExtent(support.capabilities);

    uint32_t imageCount = support.capabilities.minImageCount + 1;
    if (support.capabilities.maxImageCount > 0 && imageCount > support.capabilities.maxImageCount) {
        imageCount = support.capabilities.maxImageCount;
    }

    const QueueFamilyIndices indices = device_.findPhysicalQueueFamilies();
    const std::array<uint32_t, 2> queueFamilyIndices = {indices.graphicsFamily, indices.presentFamily};

    vk::SwapchainCreateInfoKHR createInfo;
    createInfo.setSurface(device_.surface())
        .setMinImageCount(imageCount)
        .setImageFormat(surfaceFormat.format)
        .setImageColorSpace(surfaceFormat.colorSpace)
        .setImageExtent(extent)
        .setImageArrayLayers(1)
        .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
        .setPreTransform(support.capabilities.currentTransform)
        .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
        .setPresentMode(presentMode)
        .setClipped(VK_TRUE)
        .setOldSwapchain(vk::SwapchainKHR{});

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.setImageSharingMode(vk::SharingMode::eConcurrent)
            .setQueueFamilyIndexCount(static_cast<uint32_t>(queueFamilyIndices.size()))
            .setPQueueFamilyIndices(queueFamilyIndices.data());
    } else {
        createInfo.setImageSharingMode(vk::SharingMode::eExclusive)
            .setQueueFamilyIndexCount(0)
            .setPQueueFamilyIndices(nullptr);
    }

    vk::Result result = device_.device().createSwapchainKHR(&createInfo, nullptr, &swapchain_);
    if (result != vk::Result::eSuccess) {
        std::cout << "create swapchain failed" << std::endl;
        swapchain_ = nullptr;
        return;
    }

    result = device_.device().getSwapchainImagesKHR(swapchain_, &imageCount, nullptr);
    if (result != vk::Result::eSuccess) {
        std::cout << "query swapchain image count failed" << std::endl;
        return;
    }
    swapchainImages_.resize(imageCount);
    result = device_.device().getSwapchainImagesKHR(swapchain_, &imageCount, swapchainImages_.data());
    if (result != vk::Result::eSuccess) {
        std::cout << "get swapchain images failed" << std::endl;
        swapchainImages_.clear();
        return;
    }

    swapchainImageFormat_ = surfaceFormat.format;
    swapchainExtent_ = extent;
}

void Swapchain::createImageViews() {
    swapchainImageViews_.resize(swapchainImages_.size());
    for (size_t i = 0; i < swapchainImages_.size(); ++i) {
        vk::ImageViewCreateInfo viewInfo;
        viewInfo.setImage(swapchainImages_[i])
            .setViewType(vk::ImageViewType::e2D)
            .setFormat(swapchainImageFormat_)
            .setSubresourceRange(vk::ImageSubresourceRange{}
                .setAspectMask(vk::ImageAspectFlagBits::eColor)
                .setBaseMipLevel(0)
                .setLevelCount(1)
                .setBaseArrayLayer(0)
                .setLayerCount(1));

        vk::Result result = device_.device().createImageView(&viewInfo, nullptr, &swapchainImageViews_[i]);
        if (result != vk::Result::eSuccess) {
            std::cout << "create swapchain image view failed" << std::endl;
            return;
        }
    }
}

void Swapchain::createRenderPass() {
    vk::AttachmentDescription depthAttachment;
    depthAttachment.setFormat(findDepthFormat())
        .setSamples(vk::SampleCountFlagBits::e1)
        .setLoadOp(vk::AttachmentLoadOp::eClear)
        .setStoreOp(vk::AttachmentStoreOp::eDontCare)
        .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
        .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
        .setInitialLayout(vk::ImageLayout::eUndefined)
        .setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

    vk::AttachmentReference depthAttachmentRef;
    depthAttachmentRef.setAttachment(1).setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

    vk::AttachmentDescription colorAttachment;
    colorAttachment.setFormat(swapchainImageFormat_)
        .setSamples(vk::SampleCountFlagBits::e1)
        .setLoadOp(vk::AttachmentLoadOp::eClear)
        .setStoreOp(vk::AttachmentStoreOp::eStore)
        .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
        .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
        .setInitialLayout(vk::ImageLayout::eUndefined)
        .setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

    vk::AttachmentReference colorAttachmentRef;
    colorAttachmentRef.setAttachment(0).setLayout(vk::ImageLayout::eColorAttachmentOptimal);

    vk::SubpassDescription subpass;
    subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
        .setColorAttachmentCount(1)
        .setPColorAttachments(&colorAttachmentRef)
        .setPDepthStencilAttachment(&depthAttachmentRef);

    vk::SubpassDependency dependency;
    dependency.setSrcSubpass(VK_SUBPASS_EXTERNAL)
        .setDstSubpass(0)
        .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
        .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
        .setSrcAccessMask(vk::AccessFlags{})
        .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);

    const std::array<vk::AttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
    vk::RenderPassCreateInfo renderPassInfo;
    renderPassInfo.setAttachmentCount(static_cast<uint32_t>(attachments.size()))
        .setPAttachments(attachments.data())
        .setSubpassCount(1)
        .setPSubpasses(&subpass)
        .setDependencyCount(1)
        .setPDependencies(&dependency);

    vk::Result result = device_.device().createRenderPass(&renderPassInfo, nullptr, &renderPass_);
    if (result != vk::Result::eSuccess) {
        std::cout << "create render pass failed" << std::endl;
    }
}

void Swapchain::createFramebuffers() {
    swapchainFramebuffers_.resize(imageCount());
    for (size_t i = 0; i < imageCount(); ++i) {
        const std::array<vk::ImageView, 2> attachments = {swapchainImageViews_[i], depthImageViews_[i]};
        vk::FramebufferCreateInfo framebufferInfo;
        framebufferInfo.setRenderPass(renderPass_)
            .setAttachmentCount(static_cast<uint32_t>(attachments.size()))
            .setPAttachments(attachments.data())
            .setWidth(swapchainExtent_.width)
            .setHeight(swapchainExtent_.height)
            .setLayers(1);

        vk::Result result = device_.device().createFramebuffer(&framebufferInfo, nullptr, &swapchainFramebuffers_[i]);
        if (result != vk::Result::eSuccess) {
            std::cout << "create framebuffer failed" << std::endl;
            return;
        }
    }
}

void Swapchain::createDepthResources() {
    const vk::Format depthFormat = findDepthFormat();
    depthImages_.resize(imageCount());
    depthImageMemories_.resize(imageCount());
    depthImageViews_.resize(imageCount());

    for (size_t i = 0; i < depthImages_.size(); ++i) {
        vk::ImageCreateInfo imageInfo;
        imageInfo.setImageType(vk::ImageType::e2D)
            .setExtent(vk::Extent3D{swapchainExtent_.width, swapchainExtent_.height, 1})
            .setMipLevels(1)
            .setArrayLayers(1)
            .setFormat(depthFormat)
            .setTiling(vk::ImageTiling::eOptimal)
            .setInitialLayout(vk::ImageLayout::eUndefined)
            .setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment)
            .setSamples(vk::SampleCountFlagBits::e1)
            .setSharingMode(vk::SharingMode::eExclusive)
            .setFlags({});

        device_.createImageWithInfo(imageInfo, vk::MemoryPropertyFlagBits::eDeviceLocal, depthImages_[i], depthImageMemories_[i]);
        if (!depthImages_[i] || !depthImageMemories_[i]) {
            std::cout << "create depth image or allocate depth memory failed" << std::endl;
            return;
        }

        vk::ImageViewCreateInfo viewInfo;
        viewInfo.setImage(depthImages_[i])
                .setViewType(vk::ImageViewType::e2D)
                .setFormat(depthFormat)
                .setSubresourceRange(vk::ImageSubresourceRange{}
                .setAspectMask(vk::ImageAspectFlagBits::eDepth)
                .setBaseMipLevel(0)
                .setLevelCount(1)
                .setBaseArrayLayer(0)
                .setLayerCount(1));

        vk::Result result = device_.device().createImageView(&viewInfo, nullptr, &depthImageViews_[i]);
        if (result != vk::Result::eSuccess) {
            std::cout << "create depth image view failed" << std::endl;
            return;
        }
    }
}

void Swapchain::createSyncObjects() {
    imageAvailableSemaphores_.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores_.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences_.resize(MAX_FRAMES_IN_FLIGHT);
    imagesInFlight_.resize(imageCount());

    vk::SemaphoreCreateInfo semaphoreInfo;
    vk::FenceCreateInfo fenceInfo;
    fenceInfo.setFlags(vk::FenceCreateFlagBits::eSignaled);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        vk::Result result = device_.device().createSemaphore(&semaphoreInfo, nullptr, &imageAvailableSemaphores_[i]);
        if (result != vk::Result::eSuccess) {
            std::cout << "create image-available semaphore failed" << std::endl;
            return;
        }
        result = device_.device().createSemaphore(&semaphoreInfo, nullptr, &renderFinishedSemaphores_[i]);
        if (result != vk::Result::eSuccess) {
            std::cout << "create render-finished semaphore failed" << std::endl;
            return;
        }
        result = device_.device().createFence(&fenceInfo, nullptr, &inFlightFences_[i]);
        if (result != vk::Result::eSuccess) {
            std::cout << "create in-flight fence failed" << std::endl;
            return;
        }
    }
}

vk::SurfaceFormatKHR Swapchain::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats) {
    for (const auto& format : availableFormats) {
        if (format.format == vk::Format::eB8G8R8A8Unorm && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            return format;
        }
    }
    return availableFormats.front();
}

vk::PresentModeKHR Swapchain::chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes) {
    for (const auto& mode : availablePresentModes) {
        if (mode == vk::PresentModeKHR::eMailbox) {
            std::cout << "Present mode: Mailbox" << std::endl;
            return mode;
        }
    }
    std::cout << "Present mode: V-Sync" << std::endl;
    return vk::PresentModeKHR::eFifo;
}

vk::Extent2D Swapchain::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    }

    vk::Extent2D actualExtent = windowExtent_;
    actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
    actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));
    return actualExtent;
}

vk::Format Swapchain::findDepthFormat() {
    return device_.findSupportedFormat(
        {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
        vk::ImageTiling::eOptimal,
        vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}

} // namespace vkEG
