#pragma once

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#include "ave_constants.h"
#include "ave_window.hpp"
#include "ave_device.hpp"
#include "ave_pipeline.hpp"
#include "ave_swapchain.hpp"
#include "ave_model.hpp"

namespace ave {
class AveApp {
public:
    static constexpr int WIDTH = 800;
    static constexpr int HEIGHT = 600;
    int num;

private:
    AveWindow aveWindow{WIDTH, HEIGHT, "Hello Vulkan"};
    AveDevice aveDevice{aveWindow};
    std::unique_ptr<AveSwapChain> aveSwapChain; //{aveDevice, aveWindow.getExtent()};
    std::unique_ptr<AvePipeline> avePipeline;


    // AveCamera aveCamera{aveDevice};
    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout pipelineLayout;

    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VkDescriptorSet> descriptorSets;

    std::unique_ptr<AveModel> aveModel;
    std::unique_ptr<AveModel> aveModel2;


    uint32_t mipLevels;
    VkImage textureImage;
    VkDeviceMemory textureImageMemory;

    VkImageView textureImageView;
    VkSampler textureSampler;


public:
    AveApp();
   ~AveApp();
    void run();

private:

    void createDescriptorSetLayout();
    void createPipelineLayout();
    void recreateSwapChain();
    void createPipeline();

    void createCommandBuffers();
    void freeCommandBuffers();

    void createDescriptorSets();

    void createTextureSampler();
    void createTextureImageView();
    void createTextureImage();
    void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);

    void loadModels();

    // void cleanup() {
    //     vkDestroyPipelineLayout(aveDevice.device(), pipelineLayout, nullptr);
    // }

    void recordCommandBuffer(uint32_t imageIndex);
    void drawFrame();
};
}