#pragma once

#include "vkEngine/device.hpp"


#include <vector>
#include <vulkan/vulkan.hpp>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_Z_ZERO_TO_ONE
#include <glm/glm.hpp>



namespace vkEG {
    //作用是获取顶点数据
    class Model{
        public:
            struct Vertex{
                glm::vec2 position;
                glm::vec3 color;
                static std::vector<vk::VertexInputBindingDescription> getBindingDescriputions();
                static std::vector<vk::VertexInputAttributeDescription> getAttributeDescriputions();
            };

            Model(Device& device,const std::vector<Vertex> &vertices);
            ~Model();
            //删除拷贝构造和移动构造
            Model(const Model&) = delete;
            Model& operator=(const Model&) = delete;

            void bind(vk::CommandBuffer commandBuffer);
            void draw(vk::CommandBuffer commandBuffer);

        private:
            void createVertexBuffers(const std::vector<Vertex> &vertices);
            Device& device_;
            vk::Buffer vertexBuffer_;
            vk::DeviceMemory vertexBufferMemory_;
            uint32_t vertexCount_;

    };
}