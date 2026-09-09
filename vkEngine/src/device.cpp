#include "vkEngine/device.hpp"
#include "vulkan/vulkan.hpp"


#include <iostream>
#include <set>
#include <unordered_set>

namespace vkEG {

// class member functions
Device::Device(Window &window) : window_{window} {
  createInstance();
  createSurface();
  pickPhysicalDevice();
  createLogicalDevice();
  createCommandPool();
}
Device::~Device() {
  device_.destroyCommandPool(commandPool_);
  device_.destroy();
  
  instance_.destroySurfaceKHR(surface_);
  instance_.destroy();
}

void Device::createInstance() {
  vk::ApplicationInfo appInfo;

  appInfo.setPApplicationName("VulkanEngine App")
      .setApplicationVersion(VK_MAKE_VERSION(1, 0, 0))
      .setPEngineName("No Engine")
      .setEngineVersion(VK_MAKE_VERSION(1, 0, 0))
      .setApiVersion(VK_API_VERSION_1_0);

  auto extensions = getRequiredExtensions();

  vk::InstanceCreateInfo createInfo;
  createInfo.setPApplicationInfo(&appInfo)
      .setEnabledExtensionCount(static_cast<uint32_t>(extensions.size()))
      .setPEnabledExtensionNames(extensions);

  if (enableValidationLayers) {
    createInfo
        .setEnabledLayerCount(static_cast<uint32_t>(validationLayers.size()))
        .setPEnabledLayerNames(validationLayers);
  }

  if (vk::createInstance(&createInfo, nullptr, &instance_) != vk::Result::eSuccess) {
    std::cout << "create instance failed" << std::endl;
  }

  hasGflwRequiredInstanceExtensions();
}

void Device::pickPhysicalDevice() {
  auto devices = instance_.enumeratePhysicalDevices();

  if (devices.empty()) {
    std::cout<<"find GPUs with Vulkan support failed"<<std::endl;
  }
  std::cout << "Device count: " << devices.size() << std::endl;

  for (const auto &device : devices) {
    if (isDeviceSuitable(device)) {
      physicalDevice_ = device;
      break;
    }
  }

  if (physicalDevice_ == VK_NULL_HANDLE) {
    std::cout<<"find a suitable GPU failed"<<std::endl;
  }
  properties_ = physicalDevice_.getProperties();
  std::cout << "physical device: " << properties_.deviceName << std::endl;
}

void Device::createLogicalDevice() {
  QueueFamilyIndices indices = findQueueFamilies(physicalDevice_);

  std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
  std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily,indices.presentFamily};

  float queuePriority = 1.0f;
  for (uint32_t queueFamily : uniqueQueueFamilies) {
    vk::DeviceQueueCreateInfo queueCreateInfo;

    queueCreateInfo.setQueueFamilyIndex(queueFamily)
                   .setQueueCount(1)
                   .setQueuePriorities(queuePriority);
    queueCreateInfos.push_back(queueCreateInfo);
  }

  vk::PhysicalDeviceFeatures deviceFeatures;
  deviceFeatures.setSamplerAnisotropy(true);

  vk::DeviceCreateInfo createInfo;

  createInfo.setQueueCreateInfoCount(static_cast<uint32_t>(queueCreateInfos.size()))
            .setPQueueCreateInfos(queueCreateInfos.data())
            .setPEnabledFeatures(&deviceFeatures)
            .setEnabledExtensionCount(static_cast<uint32_t>(deviceExtensions.size()))
            .setPEnabledExtensionNames(deviceExtensions);
  //被迫使用c风格的cpp函数 不然会抛异常
  vk::Device device;
  vk::Result result = physicalDevice_.createDevice(&createInfo,nullptr,&device);

  if (result != vk::Result::eSuccess) {
      std::cout << "create logical device failed"<< std::endl;
      return;
  }

  device_ = device;
  graphicsQueue_ = device_.getQueue(indices.graphicsFamily, 0);
  presentQueue_ = device_.getQueue(indices.presentFamily, 0);
}

void Device::createCommandPool() {
  QueueFamilyIndices queueFamilyIndices = findPhysicalQueueFamilies();

  vk::CommandPoolCreateInfo poolInfo;
  poolInfo.setQueueFamilyIndex(queueFamilyIndices.graphicsFamily)
          .setFlags(vk::CommandPoolCreateFlagBits::eTransient | vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
  //被迫使用c风格的cpp函数 不然会抛异常
  vk::CommandPool commandPool;
  vk::Result result = device_.createCommandPool(&poolInfo,nullptr,&commandPool);

  if (result != vk::Result::eSuccess) {
      std::cout << "create command pool failed"<< std::endl;
      return;
    }
}

void Device::createSurface() {
  window_.createWindowSurface(instance_, &surface_);
}

bool Device::isDeviceSuitable(vk::PhysicalDevice device) {
  QueueFamilyIndices indices = findQueueFamilies(device);

  bool extensionsSupported = checkDeviceExtensionSupport(device);

  bool swapChainAdequate = false;
  if (extensionsSupported) {
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
    //一些判断
    swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
  }

  vk::PhysicalDeviceFeatures supportedFeatures;
  supportedFeatures = device.getFeatures();

  return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

std::vector<const char *> Device::getRequiredExtensions() {
  uint32_t glfwExtensionCount = 0;
  const char **glfwExtensions;
  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  std::vector<const char *> extensions(glfwExtensions,
                                       glfwExtensions + glfwExtensionCount);

  return extensions;
}

void Device::hasGflwRequiredInstanceExtensions() {
  uint32_t extensionCount = 0;
  vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
  std::vector<VkExtensionProperties> extensions(extensionCount);
  vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount,
                                         extensions.data());

  std::cout << "available extensions:" << std::endl;
  std::unordered_set<std::string> available;
  for (const auto &extension : extensions) {
    std::cout << "\t" << extension.extensionName << std::endl;
    available.insert(extension.extensionName);
  }

  std::cout << "required extensions:" << std::endl;
  auto requiredExtensions = getRequiredExtensions();
  for (const auto &required : requiredExtensions) {
    std::cout << "\t" << required << std::endl;
    if (available.find(required) == available.end()) {
      throw std::runtime_error("Missing required glfw extension");
    }
  }
}

bool Device::checkDeviceExtensionSupport(vk::PhysicalDevice device) {
    auto availableExtensions = device.enumerateDeviceExtensionProperties();

    std::set<std::string> requiredExtensions(
        deviceExtensions.begin(),
        deviceExtensions.end()
    );

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

QueueFamilyIndices Device::findQueueFamilies(vk::PhysicalDevice device) {
    QueueFamilyIndices indices;

    auto queueFamilies = device.getQueueFamilyProperties();

    uint32_t i = 0;

    for (const auto& queueFamily : queueFamilies) {
            //一些判断，不太看得懂，但应该有用
        if (queueFamily.queueCount > 0 && queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
            indices.graphicsFamily = i;
            indices.graphicsFamilyHasValue = true;
        }

        auto presentSupport = device.getSurfaceSupportKHR(i, surface_);
        if (queueFamily.queueCount > 0 && presentSupport) {
            indices.presentFamily = i;
            indices.presentFamilyHasValue = true;
        }
        if (indices.isComplete()) {
            break;
        }
        i++;
    }
    return indices;
}

SwapChainSupportDetails Device::querySwapChainSupport(vk::PhysicalDevice device) {
  SwapChainSupportDetails details;
  details.capabilities = device.getSurfaceCapabilitiesKHR(surface_);

  auto format = device.getSurfaceFormatsKHR(surface_);
  if (format.empty()) {
    details.formats.resize(format.size());
    details.formats = device.getSurfaceFormatsKHR(surface_);
  }
  
  uint32_t presentModeCount;
  auto presentMode = device.getSurfacePresentModesKHR(surface_);

  if (presentMode.empty()) {
    details.presentModes.resize(presentMode.size());
    details.presentModes = device.getSurfacePresentModesKHR(surface_);
  }
  return details;
}

vk::Format Device::findSupportedFormat(const std::vector<vk::Format> &candidates,vk::ImageTiling tiling,vk::FormatFeatureFlags features) {
  for (vk::Format format : candidates) {
    vk::FormatProperties props;
    physicalDevice_.getFormatProperties(format,&props);

    if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features) {
      return format;
    }else if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features) {
      return format;
    }std::cout<<"find supported format failed"<<std::endl;
  }
  return findSupportedFormat(candidates,tiling,features);
}

uint32_t Device::findMemoryType(uint32_t typeFilter,vk::MemoryPropertyFlags properties){
  vk::PhysicalDeviceMemoryProperties memProperties = physicalDevice_.getMemoryProperties();
  for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
    if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
      return i;
    }
  }
  std::cout<<"find suitable memory type failed"<<std::endl;
  return NULL;
}

void Device::createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage,vk::MemoryPropertyFlags properties,vk::Buffer &buffer, vk::DeviceMemory &bufferMemory) {
  vk::BufferCreateInfo bufferInfo;
  bufferInfo.setSize(size)
            .setUsage(usage)
            .setSharingMode(vk::SharingMode::eExclusive);
  //防止异常 所以用c风格的cpp
  if (device_.createBuffer(&bufferInfo, nullptr, &buffer) != vk::Result::eSuccess) {
    std::cout<<"create vertex buffer failed"<<std::endl;
    return;
  }

  vk::MemoryRequirements memRequirements = device_.getBufferMemoryRequirements(buffer);

  vk::MemoryAllocateInfo allocInfo;

  allocInfo.setAllocationSize(memRequirements.size)
           .setMemoryTypeIndex(findMemoryType(memRequirements.memoryTypeBits, properties));

  if (device_.allocateMemory(&allocInfo, nullptr, &bufferMemory) != vk::Result::eSuccess) {
    std::cout<<"allocate vertex buffer memory failed"<<std::endl;
    return;
  }

  device_.bindBufferMemory(buffer, bufferMemory, 0);
}

vk::CommandBuffer Device::beginSingleTimeCommands() {
  vk::CommandBufferAllocateInfo allocInfo;
  allocInfo.setLevel(vk::CommandBufferLevel::ePrimary)
           .setCommandPool(commandPool_)
           .setCommandBufferCount(1);

  vk::CommandBuffer commandBuffer;
  if(device_.allocateCommandBuffers(&allocInfo,&commandBuffer) != vk::Result::eSuccess){
     std::cout<<"allocate command buffer failed"<<std::endl;
  }

  vk::CommandBufferBeginInfo beginInfo;
  beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
  
  commandBuffer.begin(beginInfo);
  return commandBuffer;
}

void Device::endSingleTimeCommands(vk::CommandBuffer commandBuffer) {
  commandBuffer.end();

    vk::SubmitInfo submitInfo;

    submitInfo.setCommandBufferCount(1)
              .setPCommandBuffers(&commandBuffer);

    vk::Result result = graphicsQueue_.submit(1, &submitInfo, {});

    if (result != vk::Result::eSuccess) {
        std::cout << "submit command buffer failed" << std::endl;
        return;
    }

    graphicsQueue_.waitIdle();

    device_.freeCommandBuffers(commandPool_, 1, &commandBuffer);
}

void Device::copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer,vk::DeviceSize size) {
  vk::CommandBuffer commandBuffer = beginSingleTimeCommands();

    vk::BufferCopy copyRegion{};

    copyRegion.setSize(size);

    commandBuffer.copyBuffer(srcBuffer, dstBuffer, 1, &copyRegion);

    endSingleTimeCommands(commandBuffer);
}

void Device::copyBufferToImage(vk::Buffer buffer, vk::Image image,uint32_t width, uint32_t height,uint32_t layerCount) {
      vk::CommandBuffer commandBuffer = beginSingleTimeCommands();

    vk::BufferImageCopy region{};

    region.setBufferOffset(0)
          .setBufferRowLength(0)
          .setBufferImageHeight(0)
          .setImageSubresource(
              vk::ImageSubresourceLayers{}
                  .setAspectMask(vk::ImageAspectFlagBits::eColor)
                  .setMipLevel(0)
                  .setBaseArrayLayer(0)
                  .setLayerCount(layerCount)
          )
          .setImageOffset({0, 0, 0})
          .setImageExtent({width, height, 1});

    commandBuffer.copyBufferToImage(buffer,image,vk::ImageLayout::eTransferDstOptimal,1,&region);

    endSingleTimeCommands(commandBuffer);
}

void Device::createImageWithInfo(const vk::ImageCreateInfo &imageInfo,vk::MemoryPropertyFlags properties,vk::Image &image,vk::DeviceMemory &imageMemory) {
    if (device_.createImage(&imageInfo, nullptr, &image) != vk::Result::eSuccess) {
        std::cout << "create image failed" << std::endl;
        return;
    }

    vk::MemoryRequirements memRequirements = device_.getImageMemoryRequirements(image);

    vk::MemoryAllocateInfo allocInfo;

    allocInfo.setAllocationSize(memRequirements.size)
             .setMemoryTypeIndex(findMemoryType(memRequirements.memoryTypeBits, properties));

    if (device_.allocateMemory(&allocInfo, nullptr, &imageMemory) != vk::Result::eSuccess) {
        std::cout << "allocate image memory failed" << std::endl;
        return;
    }

    device_.bindImageMemory(image, imageMemory, 0);
}

} // namespace vkEG