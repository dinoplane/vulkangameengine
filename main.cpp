#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#include "ave_constants.h"
#include "ave_window.hpp"
#include "ave_device.hpp"
#include "ave_pipeline.hpp"
#include "ave_swapchain.hpp"
#include "ave_model.hpp"


#include <algorithm>
#include <iostream>
#include <cstring>
#include <vector>
#include <stdexcept>
#include <cstdlib>
#include <optional>
#include <set>
#include <cstdint> // Necessary for uint32_t
#include <limits> // Necessary for std::numeric_limits
#include <fstream>

#include <memory>
#include <cassert>
#include <array>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;
namespace ave {


class HelloTriangleApplication {
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


public:
    HelloTriangleApplication(){
        createDescriptorSetLayout();
        createPipelineLayout();
        recreateSwapChain();
        createCommandBuffers();
        loadModels();
        createDescriptorSets();


    }
   ~HelloTriangleApplication(){



        vkDestroyDescriptorSetLayout(aveDevice.device(), descriptorSetLayout, nullptr);

        vkDestroyPipelineLayout(aveDevice.device(), pipelineLayout, nullptr);
    };
    void run() {
        while (!aveWindow.shouldClose()) {
            glfwPollEvents();
            drawFrame();
        }
        vkDeviceWaitIdle(aveDevice.device());
    }

private:
    void loadModels(){
        // const std::vector<Vertex> vertices = {
        //     {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        //     {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
        //     {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
        // };

        std::vector<Vertex> vertices;
        // FirstApp::triforce(vertices,
        //     {{0.0f, -1.0f}, {1.0, 0.0, 1.0}},
        //     {{1.0f, 1.0f}, {1.0, 1.0, 0.0}},
        //     {{-1.0f, 1.0f}, {0.0, 1.0, 1.0}},
        //     7);
        vertices.insert(vertices.end(),{
            {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
            {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
            {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
            {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}});

        const std::vector<uint16_t> indices = {
          0, 1, 2, 2, 3, 0
        };

        aveModel = std::make_unique<AveModel>(aveDevice, vertices, indices);
    }

    void createDescriptorSets(){
        std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = aveDevice.getDescriptorPool();
        allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        allocInfo.pSetLayouts = layouts.data();

        descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
        if (vkAllocateDescriptorSets(aveDevice.device(), &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = aveModel->getUniformBuffer(i);
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);

            VkWriteDescriptorSet descriptorWrite{};
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = descriptorSets[i];
            descriptorWrite.dstBinding = 0;
            descriptorWrite.dstArrayElement = 0;

            descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrite.descriptorCount = 1;

            descriptorWrite.pBufferInfo = &bufferInfo;
            descriptorWrite.pImageInfo = nullptr; // Optional
            descriptorWrite.pTexelBufferView = nullptr; // Optional

            vkUpdateDescriptorSets(aveDevice.device(), 1, &descriptorWrite, 0, nullptr);


        }
   }

    void createDescriptorSetLayout() {
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.descriptorCount = 1;

        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = 1;
        layoutInfo.pBindings = &uboLayoutBinding;

        if (vkCreateDescriptorSetLayout(aveDevice.device(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }

    void createPipelineLayout(){
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
        pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
        pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

        if (vkCreatePipelineLayout(aveDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    }

    void createPipeline(){
        assert(aveSwapChain != nullptr && "Cannot create pipeline before swapchain");
        assert(pipelineLayout != nullptr && "Cannot create pipeline before layout");

        ave::PipelineConfigInfo pipelineConfig{};
        AvePipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = aveSwapChain->getRenderPass();
        pipelineConfig.pipelineLayout = pipelineLayout;
        avePipeline = std::make_unique<AvePipeline>(
            aveDevice,
            "shaders/shader.vert.spv",
            "shaders/shader.frag.spv",
            pipelineConfig);
    };

    void recreateSwapChain(){
        auto extent = aveWindow.getExtent();
        while (extent.width == 0 || extent.height == 0){
            extent = aveWindow.getExtent();
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(aveDevice.device());

        if (aveSwapChain == nullptr)
            aveSwapChain = std::make_unique<AveSwapChain>(aveDevice, extent);
        else {
            aveSwapChain = std::make_unique<AveSwapChain>(aveDevice, extent, std::move(aveSwapChain));
            if (aveSwapChain->imageCount() != commandBuffers.size()){
                freeCommandBuffers();
                createCommandBuffers();
            }
        }

        // If render passes are compatible, then do nothing...(no need to be recreated)
        createPipeline();
    }

    void createCommandBuffers() {
        commandBuffers.resize(aveSwapChain->imageCount());
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = aveDevice.getCommandPool();
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t) commandBuffers.size();

        if (vkAllocateCommandBuffers(aveDevice.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }

    }

    void freeCommandBuffers(){
        vkFreeCommandBuffers(
            aveDevice.device(),
            aveDevice.getCommandPool(),
            static_cast<u_int32_t>(commandBuffers.size()),
            commandBuffers.data());
        commandBuffers.clear();
    }

    // void cleanup() {
    //     vkDestroyPipelineLayout(aveDevice.device(), pipelineLayout, nullptr);
    // }

    void recordCommandBuffer(uint32_t imageIndex) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0; // Optional
        beginInfo.pInheritanceInfo = nullptr; // Optional

        if (vkBeginCommandBuffer(commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }
        aveModel->updateUniformBuffer(aveSwapChain->getCurrentFrame(), aveSwapChain->getSwapChainExtent());


        // Begin Drawing

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = aveSwapChain->getRenderPass();
        renderPassInfo.framebuffer = aveSwapChain->getFrameBuffer(imageIndex);
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = aveSwapChain->getSwapChainExtent();

        VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        // vkCmdBindPipeline(commandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
        avePipeline->bind(commandBuffers[imageIndex]);
        aveModel->bind(commandBuffers[imageIndex]);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(aveSwapChain->getSwapChainExtent().width);
        viewport.height = static_cast<float>(aveSwapChain->getSwapChainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffers[imageIndex], 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = aveSwapChain->getSwapChainExtent();
        vkCmdSetScissor(commandBuffers[imageIndex], 0, 1, &scissor);

        // vkCmdDraw(commandBuffers[imageIndex], 3, 1, 0, 0);
        vkCmdBindDescriptorSets(commandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[aveSwapChain->getCurrentFrame()], 0, nullptr);
        aveModel->draw(commandBuffers[imageIndex]);


        vkCmdEndRenderPass(commandBuffers[imageIndex]);

        if (vkEndCommandBuffer(commandBuffers[imageIndex]) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }

    void drawFrame() {
        u_int32_t imageIndex;
        auto result = aveSwapChain->acquireNextImage(&imageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR){
            recreateSwapChain();
            return;
        }

        if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR){
            throw std::runtime_error("failed to acquire swap chain image");
        }

        recordCommandBuffer(imageIndex);
        result = aveSwapChain->submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || aveWindow.wasWindowResized()){
            aveWindow.resetWindowResizedFlag();
            recreateSwapChain();
            return;
        }

        if (result != VK_SUCCESS){
            throw std::runtime_error("failed to present swap chain image");
        }
    }
};

}

int main() {
    ave::HelloTriangleApplication app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

