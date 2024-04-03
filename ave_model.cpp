#include "ave_model.hpp"

namespace ave {
    AveModel::AveModel(AveDevice& device, const std::vector<Vertex>& vertices, const std::vector<u_int16_t>& indices) : aveDevice{device} {
        createVertexBuffers(vertices);
        createIndexBuffer(indices);
        createUniformBuffers();
    }

    AveModel::~AveModel(){
        vkDestroyBuffer(aveDevice.device(), indexBuffer, nullptr);
        vkFreeMemory(aveDevice.device(), indexBufferMemory, nullptr);

        vkDestroyBuffer(aveDevice.device(), vertexBuffer, nullptr); // RAII
        vkFreeMemory(aveDevice.device(), vertexBufferMemory, nullptr);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroyBuffer(aveDevice.device(), uniformBuffers[i], nullptr);
            vkFreeMemory(aveDevice.device(), uniformBuffersMemory[i], nullptr);
        }

    }

    void AveModel::createVertexBuffers(const std::vector<Vertex> &vertices){
        vertexCount = static_cast<uint32_t>(vertices.size());
        assert(vertexCount >= 3 && "Vertex Count must be greater than 3");
        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        aveDevice.createBuffer(
            bufferSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer,
            stagingBufferMemory);




        void* data;

        vkMapMemory(aveDevice.device(), stagingBufferMemory, 0, bufferSize, 0, &data);

        memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
        vkUnmapMemory(aveDevice.device(), stagingBufferMemory);

        aveDevice.createBuffer(
            bufferSize,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            vertexBuffer,
            vertexBufferMemory
        );

        aveDevice.copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

        vkDestroyBuffer(aveDevice.device(), stagingBuffer, nullptr);
        vkFreeMemory(aveDevice.device(), stagingBufferMemory, nullptr);
    }

    void AveModel::createIndexBuffer(const std::vector<u_int16_t> &indices) {
        indexCount = static_cast<u_int32_t>(indices.size());
        VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        aveDevice.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* data;
        vkMapMemory(aveDevice.device(), stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, indices.data(), (size_t) bufferSize);
        vkUnmapMemory(aveDevice.device(), stagingBufferMemory);

        aveDevice.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

        aveDevice.copyBuffer(stagingBuffer, indexBuffer, bufferSize);

        vkDestroyBuffer(aveDevice.device(), stagingBuffer, nullptr);
        vkFreeMemory(aveDevice.device(), stagingBufferMemory, nullptr);
    }


    void AveModel::createUniformBuffers() {
        VkDeviceSize bufferSize = sizeof(UniformBufferObject);

        uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
        uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            aveDevice.createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBuffersMemory[i]);

            vkMapMemory(aveDevice.device(), uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
        }
    }

    void AveModel::draw(VkCommandBuffer commandBuffer){
        // vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);

        vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
    }

    void AveModel::bind(VkCommandBuffer commandBuffer){
        VkBuffer buffers[] = {vertexBuffer};

        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

        vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT16);
    }

void AveModel::updateUniformBuffer(uint32_t currentImage, VkExtent2D swapChainExtent) {
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

UniformBufferObject ubo{};
ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float) swapChainExtent.height, 0.1f, 10.0f);

ubo.proj[1][1] *= -1;

memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));

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