#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "ave_constants.h"

#include "ave_window.hpp"
#include <string>
#include <vector>
#include <array>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <optional>
#include <set>

namespace ave {

    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData);


    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete() {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };



    class AveDevice {
        const bool enableValidationLayers = false;

        const std::vector<const char*> validationLayers = {
            "VK_LAYER_KHRONOS_validation"
        };

        const std::vector<const char*> deviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };
        public:
            int noop;
        private:
            VkInstance instance;
            AveWindow& window;
            VkDebugUtilsMessengerEXT debugMessenger;
            VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
            VkCommandPool commandPool;
            VkDescriptorPool descriptorPool;

            VkDevice device_; // logical device
            VkSurfaceKHR surface_;
            VkQueue graphicsQueue_;
            VkQueue presentQueue_;

            VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;

        public:
            AveDevice(AveWindow& window);
            ~AveDevice();

            AveDevice(const AveDevice&) = delete;
            AveDevice& operator=(const AveDevice) = delete;
            AveDevice(AveDevice &&) = delete;
            AveDevice &operator=(AveDevice &&) = delete;

            VkCommandPool getCommandPool() { return commandPool; }
            VkDescriptorPool getDescriptorPool() { return descriptorPool; }
            VkDevice device() { return device_; }
            VkPhysicalDevice getPhysicalDevice() { return physicalDevice; }
            VkSurfaceKHR surface() { return surface_; }
            VkQueue graphicsQueue() { return graphicsQueue_; }
            VkQueue presentQueue() { return presentQueue_; }
            VkSampleCountFlagBits getMsaaSamples() { return msaaSamples; }


            bool hasStencilComponent(VkFormat format) {
                return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
            }

            SwapChainSupportDetails getSwapChainSupport() { return querySwapChainSupport(physicalDevice); }
            uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
            QueueFamilyIndices findPhysicalQueueFamilies() { return findQueueFamilies(physicalDevice); }
            VkFormat findSupportedFormat(
                const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
            VkFormat findDepthFormat();
            // Buffer Helper Functions
            void createBuffer(
                VkDeviceSize size,
                VkBufferUsageFlags usage,
                VkMemoryPropertyFlags properties,
                VkBuffer &buffer,
                VkDeviceMemory &bufferMemory);
            VkCommandBuffer beginSingleTimeCommands();
            void endSingleTimeCommands(VkCommandBuffer commandBuffer);
            void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
            void copyBufferToImage(
                VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount);
            void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
            void createImageWithInfo(
                const VkImageCreateInfo &imageInfo,
                VkMemoryPropertyFlags properties,
                VkImage &image,
                VkDeviceMemory &imageMemory);
            void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

        private:
            void createInstance();
            void setupDebugMessenger();
            void createSurface();
            void pickPhysicalDevice();
            void createLogicalDevice();
            void createCommandPool();
            // void createDepthResources();
            void createDescriptorPool();

            // helper functions
            VkSampleCountFlagBits getMaxUsableSampleCount();
            bool checkValidationLayerSupport();
            void populateDebugUtilMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
            std::vector<const char*> getRequiredExtensions();
            bool isDeviceSuitable(VkPhysicalDevice device);
            bool checkDeviceExtensionSupport(VkPhysicalDevice device);
            QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
            SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);


    };
}

