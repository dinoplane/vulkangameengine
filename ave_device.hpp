#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "ave_window.hpp"
#include <string>
#include <vector>
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
        const bool enableValidationLayers = true;

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

            VkDevice device_; // logical device
            VkSurfaceKHR surface_;
            VkQueue graphicsQueue_;
            VkQueue presentQueue_;

        public:
            AveDevice(AveWindow& window);
            ~AveDevice();

            AveDevice(const AveDevice&) = delete;
            AveDevice& operator=(const AveDevice) = delete;
            AveDevice(AveDevice &&) = delete;
            AveDevice &operator=(AveDevice &&) = delete;

            VkCommandPool getCommandPool() { return commandPool; }
            VkDevice device() { return device_; }
            VkSurfaceKHR surface() { return surface_; }
            VkQueue graphicsQueue() { return graphicsQueue_; }
            VkQueue presentQueue() { return presentQueue_; }

            SwapChainSupportDetails getSwapChainSupport() { return querySwapChainSupport(physicalDevice); }
            // uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
            QueueFamilyIndices findPhysicalQueueFamilies() { return findQueueFamilies(physicalDevice); }
            // VkFormat findSupportedFormat(
            //     const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features);


        private:
            void createInstance();
            void setupDebugMessenger();
            void createSurface();
            void pickPhysicalDevice();
            void createLogicalDevice();
            void createCommandPool();

            // helper functions
            bool checkValidationLayerSupport();
            void populateDebugUtilMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
            std::vector<const char*> getRequiredExtensions();
            bool isDeviceSuitable(VkPhysicalDevice device);
            bool checkDeviceExtensionSupport(VkPhysicalDevice device);
            QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
            SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);


    };
}

