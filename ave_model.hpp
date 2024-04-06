#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "ave_constants.h"
#include "ave_device.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <array>
#include <vector>
#include <cassert>
#include <cstring>

#include <chrono>

namespace ave {
    struct Vertex {
        glm::vec3 pos;
        glm::vec3 color;
        glm::vec2 texCoord;

        static VkVertexInputBindingDescription getBindingDescription() {
            VkVertexInputBindingDescription bindingDescription{};
            bindingDescription.binding = 0;
            bindingDescription.stride = sizeof(Vertex);
            bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // VK_VERTEX_INPUT_RATE_INSTANCE for instanced rendering
            return bindingDescription;
        }


        static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
            std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};
            attributeDescriptions[0].binding = 0;
            attributeDescriptions[0].location = 0;
            attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[0].offset = offsetof(Vertex, pos);

            attributeDescriptions[1].binding = 0;
            attributeDescriptions[1].location = 1;
            attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[1].offset = offsetof(Vertex, color);

            attributeDescriptions[2].binding = 0;
            attributeDescriptions[2].location = 2;
            attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
            attributeDescriptions[2].offset = offsetof(Vertex, texCoord);


            return attributeDescriptions;
        }

        Vertex operator+(const Vertex& other){
            return {pos+other.pos, color+other.color};
        }
        Vertex operator-(const Vertex& other){
            return {pos - other.pos, color - other.color};
        }
        Vertex operator/(float f){
            return {pos / f, color / f};
        }
        Vertex operator*(float f){
            return {pos * f, color * f};
        }
    };

    struct UniformBufferObject {
        alignas(16) glm::mat4 model;
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 proj;
    };


    class AveModel {
        public:
            AveModel(AveDevice& device, const std::vector<Vertex> &vertices, const std::vector<u_int16_t> &indices);
            ~AveModel();

            AveModel(const AveModel&) = delete;
            AveModel& operator=(const AveModel&) = delete;

            void bind(VkCommandBuffer commandBuffer);
            void updateUniformBuffer(uint32_t currentImage, VkExtent2D swapChainExtent);
            void draw(VkCommandBuffer commandBuffer);

            VkBuffer& getUniformBuffer(size_t i) { return uniformBuffers[i]; }

        private:
            void createVertexBuffers(const std::vector<Vertex> &vertices);
            void createIndexBuffer(const std::vector<u_int16_t>& indices);
            void createUniformBuffers();
            AveDevice& aveDevice;
            VkBuffer vertexBuffer;
            VkDeviceMemory vertexBufferMemory;
            uint32_t vertexCount;

            VkBuffer indexBuffer;
            VkDeviceMemory indexBufferMemory;
            u_int32_t indexCount;

            std::vector<VkBuffer> uniformBuffers;
            std::vector<VkDeviceMemory> uniformBuffersMemory;
            std::vector<void*> uniformBuffersMapped;


    };

}