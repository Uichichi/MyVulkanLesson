#include "vkEngine/model.hpp"
#include "vulkan/vulkan.hpp"
#include <cassert>
#include <cstdint>
#include <cstring>
#include <vector>

namespace vkEG {
    Model::Model(Device& device,const std::vector<Vertex> &vertices) : device_{device}{
        createVertexBuffers(vertices);

    };
    Model::~Model(){
        //后期会使用vma内存分配器，也可以自己实现，参考Kyle Halliday的博客
        // https://kylehalladay.com/blog/tutorial/2017/12/13/Custom-Allocators-Vulkan.html
        device_.device().destroyBuffer(vertexBuffer_);
        device_.device().freeMemory(vertexBufferMemory_);
    };
    void Model::createVertexBuffers(const std::vector<Vertex> &vertices){
        vertexCount_ = static_cast<uint32_t>(vertices.size());
        assert(vertices.size() >= 3 && "Vertex count must be at least 3");
        vk::DeviceSize bufferSize = sizeof(vertices[0]) * vertexCount_;

        device_.createBuffer(bufferSize, vk::BufferUsageFlagBits::eVertexBuffer , vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, vertexBuffer_, vertexBufferMemory_); 
        //将data映射到gpu的顶点缓冲区内存起始处
        void* data = device_.device().mapMemory(vertexBufferMemory_, 0 ,bufferSize, vk::MemoryMapFlags{});
        memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
        device_.device().unmapMemory(vertexBufferMemory_);
    };
    void Model::draw(vk::CommandBuffer commandBuffer){
        commandBuffer.draw(vertexCount_, 1, 0, 0); 
    };
    void Model::bind(vk::CommandBuffer commandBuffer){
        vk::Buffer buffers[] = {vertexBuffer_};
        vk::DeviceSize offsets[] = {0};
        commandBuffer.bindVertexBuffers(0,1,buffers,offsets);
    };
    std::vector<vk::VertexInputBindingDescription> Model::Vertex::getBindingDescriputions(){
        std::vector<vk::VertexInputBindingDescription> bindingDescriputions(1);
        bindingDescriputions[0].binding = 0;
        bindingDescriputions[0].stride = sizeof(Vertex);
        bindingDescriputions[0].inputRate = vk::VertexInputRate::eVertex;
        return bindingDescriputions;
    };
    std::vector<vk::VertexInputAttributeDescription> Model::Vertex::getAttributeDescriputions(){
        std::vector<vk::VertexInputAttributeDescription> attributeDescriputions(1);
        attributeDescriputions[0].binding = 0;
        attributeDescriputions[0].location = 0;
        attributeDescriputions[0].format = vk::Format::eR32G32Sfloat;
        attributeDescriputions[0].offset = 0;
        return attributeDescriputions;
    };
}