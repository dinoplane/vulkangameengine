#pragma once


#include "ave_device.hpp"


#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"


#include <vector>


namespace ave{
    class AveModel {

        public:
            struct Vertex {
                glm::vec2 position;
                glm::vec3 color;

                static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
                static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

                Vertex operator+(const Vertex& other){
                    return {position+other.position, color+other.color};
                }
                Vertex operator-(const Vertex& other){
                    return {position - other.position, color - other.color};
                }
                Vertex operator/(float f){
                    return {position / f, color / f};
                }
                Vertex operator*(float f){
                    return {position * f, color * f};
                }

            };

        public:
            AveModel(AveDevice& device, const std::vector<Vertex> &vertices);
            ~AveModel();

            AveModel(const AveModel&) = delete;
            AveModel& operator=(const AveModel&) = delete;

            void bind(VkCommandBuffer commandBuffer);
            void draw(VkCommandBuffer commandBuffer);

        private:
            void createVertexBuffers(const std::vector<Vertex> &vertices);

            AveDevice& aveDevice;
            VkBuffer vertexBuffer;
            VkDeviceMemory vertexBufferMemory;
            uint32_t vertexCount;
    };
}