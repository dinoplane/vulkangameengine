#include "ave_model.hpp"

namespace ave {
    AveModel::AveModel(AveDevice& device, const std::vector<Vertex>& vertices) : aveDevice{device} {
        createVertexBuffers(vertices);
    }

    AveModel::~AveModel(){
        vkDestroyBuffer(aveDevice.device(), vertexBuffer, nullptr); // RAII
        vkFreeMemory(aveDevice.device(), vertexBufferMemory, nullptr);
    }

    void AveModel::createVertexBuffers(const std::vector<Vertex> &vertices){
        vertexCount = static_cast<uint32_t>(vertices.size());
        assert(vertexCount >= 3 && "Vertex Count must be greater than 3");
        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;

        aveDevice.createBuffer(
            bufferSize,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            vertexBuffer,
            vertexBufferMemory
        );

        void* data;

        vkMapMemory(aveDevice.device(), vertexBufferMemory, 0, bufferSize, 0, &data);

        memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
        vkUnmapMemory(aveDevice.device(), vertexBufferMemory);
    }

    void AveModel::draw(VkCommandBuffer commandBuffer){
        vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
    }

    void AveModel::bind(VkCommandBuffer commandBuffer){
        VkBuffer buffers[] = {vertexBuffer};

        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
    }

    // std::vector<VkVertexInputBindingDescription> Vertex::getBindingDescription(){
    //     std::vector<VkVertexInputBindingDescription> bingingDesctiptions{1};
    //     bingingDesctiptions[0].binding = 0;
    //     bingingDesctiptions[0].stride = sizeof(Vertex);
    //     bingingDesctiptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    //     return bingingDesctiptions;
    // }

    // std::vector<VkVertexInputAttributeDescription> Vertex::getAttributeDescriptions(){
    //     std::vector<VkVertexInputAttributeDescription> attributeDescriptions{2};
    //     attributeDescriptions[0].binding = 0;
    //     attributeDescriptions[0].location = 0;
    //     attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
    //     attributeDescriptions[0].offset = offsetof(Vertex, position);

    //     attributeDescriptions[1].binding = 0;
    //     attributeDescriptions[1].location = 1;
    //     attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    //     attributeDescriptions[1].offset = offsetof(Vertex, color);
    //     return attributeDescriptions;
    // }

}