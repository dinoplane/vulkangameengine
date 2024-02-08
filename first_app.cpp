#include "first_app.hpp"

#include <stdexcept>

namespace ave {
    FirstApp::FirstApp(){
        loadModels();
        createPipelineLayout();
        recreateSwapChain();
        createCommandBuffers();
    };

    FirstApp::~FirstApp(){
        vkDestroyPipelineLayout(aveDevice.device(), pipelineLayout, nullptr);
    };

    void FirstApp::run(){
        while (!aveWindow.shouldClose()){
            glfwPollEvents();
            drawFrame();
        }
        vkDeviceWaitIdle(aveDevice.device()); // Block until all GPU ops are done
    };

    void FirstApp::triforce(std::vector<AveModel::Vertex> &v, AveModel::Vertex v1, AveModel::Vertex v2, AveModel::Vertex v3, float n){
        if (n == 0){
            v.push_back(v1);
            v.push_back(v2);
            v.push_back(v3);
            return;
        }
        AveModel::Vertex m12 = (v1 + v2) / 2.0f;
        AveModel::Vertex m23 = (v2 + v3) / 2.0f;
        AveModel::Vertex m13 = (v1 + v3) / 2.0f;

        triforce(v, v1, m12, m13, n - 1);
        triforce(v, m12, v2, m23, n - 1);
        triforce(v, m13, m23, v3, n - 1);
    };

    void FirstApp::loadModels(){
        std::vector<AveModel::Vertex> vertices;
        FirstApp::triforce(vertices,
            {{0.0f, -1.0f}, {1.0, 0.0, 1.0}},
            {{1.0f, 1.0f}, {1.0, 1.0, 0.0}},
            {{-1.0f, 1.0f}, {0.0, 1.0, 1.0}},
            9);

        aveModel = std::make_unique<AveModel>(aveDevice, vertices);
    };

    void FirstApp::createPipelineLayout(){
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = nullptr;
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges = nullptr;
        if (vkCreatePipelineLayout(aveDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS){
            throw std::runtime_error("failed to create pipeline layout");
        }
    };

    void FirstApp::createPipeline(){
        assert(aveSwapChain != nullptr && "Cannot create pipeline before swapchain");
        assert(pipelineLayout != nullptr && "Cannot create pipeline before layout");

        PipelineConfigInfo pipelineConfig{};
        AvePipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = aveSwapChain->getRenderPass();
        pipelineConfig.pipelineLayout = pipelineLayout;
        avePipeline = std::make_unique<AvePipeline>(
            aveDevice,
            "shaders/simple_shader.vert.spv",
            "shaders/simple_shader.frag.spv",
            pipelineConfig);
    };

    void FirstApp::recreateSwapChain(){
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

    void FirstApp::createCommandBuffers(){
        commandBuffers.resize(aveSwapChain->imageCount());

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = aveDevice.getCommandPool();
        allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

        if (vkAllocateCommandBuffers(aveDevice.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS){
            throw std::runtime_error("failed to allocate command buffers");
        }
        //aveDevice, aveWindow.getExtent()
        /*
        for (int i = 0; i < commandBuffers.size(); i++){

        }
        */

    };

    void FirstApp::recordCommandBuffer(int imageIndex){
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS){
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = aveSwapChain->getRenderPass();
        renderPassInfo.framebuffer = aveSwapChain->getFrameBuffer(imageIndex);
        renderPassInfo.renderArea.offset = {0,0};
        renderPassInfo.renderArea.extent = aveSwapChain->getSwapChainExtent();

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {0.1f, 0.1f, 0.1f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};
        renderPassInfo.clearValueCount = static_cast<u_int32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(aveSwapChain->getSwapChainExtent().width);
        viewport.height = static_cast<float>(aveSwapChain->getSwapChainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{{0, 0}, aveSwapChain->getSwapChainExtent()};
        vkCmdSetViewport(commandBuffers[imageIndex], 0, 1, &viewport);
        vkCmdSetScissor(commandBuffers[imageIndex], 0, 1, &scissor);


        avePipeline->bind(commandBuffers[imageIndex]);
        aveModel->bind(commandBuffers[imageIndex]);
        aveModel->draw(commandBuffers[imageIndex]);

        // vkCmdDraw(commandBuffers[imageIndex], 3, 1, 0, 0);// 3 vertices, 1 instance, no offsets

        vkCmdEndRenderPass(commandBuffers[imageIndex]);
        if (vkEndCommandBuffer(commandBuffers[imageIndex]) != VK_SUCCESS){
            throw std::runtime_error("failed to record command buffer");
        }
    };

    void FirstApp::freeCommandBuffers(){
        vkFreeCommandBuffers(
            aveDevice.device(),
            aveDevice.getCommandPool(),
            static_cast<u_int32_t>(commandBuffers.size()),
            commandBuffers.data());
        commandBuffers.clear();
    }

    void FirstApp::drawFrame(){
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
    };
}