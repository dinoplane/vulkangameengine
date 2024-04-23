#include "ave_app.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobjloader/tiny_obj_loader.h>

namespace ave{
    AveApp::AveApp(){
        createDescriptorSetLayout();
        createPipelineLayout();
        recreateSwapChain();
        createCommandBuffers();
        createTextureImage();
        createTextureImageView();
        createTextureSampler();
        loadModels();
        createDescriptorSets();
    }

    AveApp::~AveApp(){
        vkDestroySampler(aveDevice.device(), textureSampler, nullptr);
        vkDestroyImageView(aveDevice.device(), textureImageView, nullptr);
        vkDestroyImage(aveDevice.device(), textureImage, nullptr);
        vkFreeMemory(aveDevice.device(), textureImageMemory, nullptr);

        vkDestroyDescriptorSetLayout(aveDevice.device(), descriptorSetLayout, nullptr);
        vkDestroyPipelineLayout(aveDevice.device(), pipelineLayout, nullptr);
    };

    void AveApp::run() {
        while (!aveWindow.shouldClose()) {
            glfwPollEvents();
            drawFrame();
        }
        vkDeviceWaitIdle(aveDevice.device());
    }

    void AveApp::createDescriptorSetLayout(){
        {
            VkDescriptorSetLayoutBinding uboLayoutBinding{};
            uboLayoutBinding.binding = 0;
            uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            uboLayoutBinding.descriptorCount = 1;

            uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
            uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

            VkDescriptorSetLayoutBinding samplerLayoutBinding{};
            samplerLayoutBinding.binding = 1;
            samplerLayoutBinding.descriptorCount = 1;
            samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            samplerLayoutBinding.pImmutableSamplers = nullptr;
            samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

            std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};
            VkDescriptorSetLayoutCreateInfo layoutInfo{};
            layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
            layoutInfo.pBindings = bindings.data();

            if (vkCreateDescriptorSetLayout(aveDevice.device(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
                throw std::runtime_error("failed to create descriptor set layout!");
            }
        }
    }

    void AveApp::createPipelineLayout(){
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

    void AveApp::recreateSwapChain(){
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

    void AveApp::createPipeline(){
        assert(aveSwapChain != nullptr && "Cannot create pipeline before swapchain");
        assert(pipelineLayout != nullptr && "Cannot create pipeline before layout");

        ave::PipelineConfigInfo pipelineConfig{};
        AvePipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.multisampleInfo.rasterizationSamples = aveDevice.getMsaaSamples();
        pipelineConfig.renderPass = aveSwapChain->getRenderPass();
        pipelineConfig.pipelineLayout = pipelineLayout;
        avePipeline = std::make_unique<AvePipeline>(
            aveDevice,
            "shaders/shader.vert.spv",
            "shaders/shader.frag.spv",
            pipelineConfig);
    };

    void AveApp::createCommandBuffers() {
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

    void AveApp::freeCommandBuffers(){
        vkFreeCommandBuffers(
            aveDevice.device(),
            aveDevice.getCommandPool(),
            static_cast<u_int32_t>(commandBuffers.size()),
            commandBuffers.data());
        commandBuffers.clear();
    }

    void AveApp::createDescriptorSets(){
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

            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = textureImageView;
            imageInfo.sampler = textureSampler;


            std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = descriptorSets[i];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &bufferInfo;

            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[1].dstSet = descriptorSets[i];
            descriptorWrites[1].dstBinding = 1;
            descriptorWrites[1].dstArrayElement = 0;
            descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[1].descriptorCount = 1;
            descriptorWrites[1].pImageInfo = &imageInfo;

            vkUpdateDescriptorSets(aveDevice.device(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }
   }

    void AveApp::createTextureSampler() {
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;

        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(aveDevice.getPhysicalDevice(), &properties);

        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;

        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0; //static_cast<float>(mipLevels / 2);
        samplerInfo.maxLod = VK_LOD_CLAMP_NONE;


        if (vkCreateSampler(aveDevice.device(), &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture sampler!");
        }

    }

    void AveApp::createTextureImageView() {
        textureImageView = aveSwapChain->createImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);
    }

    void AveApp::createTextureImage(){
        int texWidth, texHeight, texChannels;
            // stbi_uc* pixels = stbi_load("textures/texture.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
            stbi_uc* pixels = stbi_load(TEXTURE_PATH.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
            mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

            VkDeviceSize imageSize = texWidth * texHeight * 4;

            if (!pixels) {
                throw std::runtime_error("failed to load texture image!");
            }

            VkBuffer stagingBuffer;
            VkDeviceMemory stagingBufferMemory;

            aveDevice.createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

            void* data;
            vkMapMemory(aveDevice.device(), stagingBufferMemory, 0, imageSize, 0, &data);
                memcpy(data, pixels, static_cast<size_t>(imageSize));
            vkUnmapMemory(aveDevice.device(), stagingBufferMemory);

            stbi_image_free(pixels);
            aveDevice.createImage(texWidth, texHeight, mipLevels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

            aveDevice.transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);
            aveDevice.copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 1);
            // aveDevice.transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);



            vkDestroyBuffer(aveDevice.device(), stagingBuffer, nullptr);
            vkFreeMemory(aveDevice.device(), stagingBufferMemory, nullptr);

            generateMipmaps(textureImage, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, mipLevels);

    }

    void AveApp::loadModels(){
            // const std::vector<Vertex> vertices = {
            //     {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
            //     {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
            //     {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
            // };



            // std::vector<Vertex> vertices;
            // std::vector<Vertex> vertices2;

            // FirstApp::triforce(vertices,
            //     {{0.0f, -1.0f}, {1.0, 0.0, 1.0}},
            //     {{1.0f, 1.0f}, {1.0, 1.0, 0.0}},
            //     {{-1.0f, 1.0f}, {0.0, 1.0, 1.0}},
            //     7);
            // vertices2.insert(vertices2.end(),{
            //     {{-0.5f, -0.5f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
            //     {{0.5f, -0.5f, 1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
            //     {{0.5f, 0.5f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
            //     {{-0.5f, 0.5f, 1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

            //     {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
            //     {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
            //     {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
            //     {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
            // });

            // std::vector<uint32_t> indices;// = {
            // //     0, 1, 2, 2, 3, 0,
            // //     4, 5, 6, 6, 7, 4
            // // };


            // std::vector<uint32_t> indices2 = {
            //     0, 1, 2,
            //     4, 5, 6
            // };


            // size_t NUM_FACE_EDGES = 36;
            // size_t NUM_HEIGHT = 5;
            // float delta_theta = 2*M_PI / NUM_FACE_EDGES;


            // for (size_t i = 0; i < NUM_FACE_EDGES; i++){

            //     Vertex vertex{};

            //     vertex.pos = {
            //         0.5f * cos(i * delta_theta),
            //         0.5f * sin(i * delta_theta),
            //         0.0f
            //     };

            //     vertex.texCoord = {
            //         0.5f + 0.5f * cos(i * delta_theta),
            //         0.5f + 0.5f * sin(i * delta_theta)
            //     };

            //     vertex.color = {1.0f, 1.0f, 1.0f};

            //     vertices.push_back(vertex);
            // }

            // Vertex vertex{};
            // vertices.push_back(vertex);

            // for (size_t i = 0; i < NUM_FACE_EDGES - 1; i++){
            //     indices.push_back(NUM_FACE_EDGES - 1);
            //     indices.push_back(i);
            //     indices.push_back((i+1) % (NUM_FACE_EDGES - 1));
            // }

            // size_t RESOLUTION = 36;
            // size_t NUM_HEIGHT = 5;
            // float start_height = -0.3f;
            // float delta_z = 1.0f / static_cast<float>(RESOLUTION);
            // float delta_theta = 2*M_PI / static_cast<float>(RESOLUTION);

            // for (size_t j = 0; j < RESOLUTION; j++) {
            //     float z =  (j * delta_z);
            //     float r =  0.5*sin(M_PI * j / RESOLUTION);
            //     for (size_t i = 0; i < RESOLUTION; i++){

            //         Vertex vertex{};


            //         vertex.pos = {
            //             r * cos(M_PI * 2 * i / RESOLUTION ),
            //             r * sin(M_PI * 2 * i / RESOLUTION ),
            //             start_height + 0.5*cos(M_PI * j / RESOLUTION)
            //         };

            //         vertex.texCoord = {
            //             0.5f + 0.5f * cos(i * delta_theta),
            //             0.5f + 0.5f * sin(i * delta_theta)
            //         };

            //         vertex.color = {1.0f, 1.0f, 1.0f};

            //         vertices.push_back(vertex);

            //     }
            // }

            // for (size_t j = 0; j < RESOLUTION - 1; j++) {
            //     for (size_t i = 0; i < RESOLUTION; i++){
            //         indices.push_back(j*RESOLUTION + i);
            //         indices.push_back(j*RESOLUTION + ((i+1) % RESOLUTION));
            //         indices.push_back((j+1)*RESOLUTION + i);

            //         indices.push_back((j+1)*RESOLUTION + i);
            //         indices.push_back(j*RESOLUTION + ((i+1) % RESOLUTION));
            //         indices.push_back((j+1)*RESOLUTION + ((i+1) % RESOLUTION));
            //     }
            // }

            // tinyobj::attrib_t attrib;
            // std::vector<tinyobj::shape_t> shapes;
            // std::vector<tinyobj::material_t> materials;
            // std::string warn, err;

            // if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, MODEL_PATH.c_str())) {
            //     throw std::runtime_error(warn + err);
            // }


            // std::unordered_map<ave::Vertex, uint32_t, ave::Vertex_hash> uniqueVertices;


            // for (const auto& shape : shapes) {
            //     for (const auto& index : shape.mesh.indices) {
            //         Vertex vertex{};

            //         vertex.pos = {
            //             attrib.vertices[3 * index.vertex_index + 0],
            //             attrib.vertices[3 * index.vertex_index + 1],
            //             attrib.vertices[3 * index.vertex_index + 2]
            //         };

            //         vertex.texCoord = {
            //             attrib.texcoords[2 * index.texcoord_index + 0],
            //             1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
            //         };

            //         vertex.color = {1.0f, 1.0f, 1.0f};

            //         if (uniqueVertices.count(vertex) == 0) {
            //             uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
            //             vertices.push_back(vertex);
            //         }

            //         indices.push_back(uniqueVertices[vertex]);
            //     }
            // }

            // aveModel = std::make_unique<AveModel>(aveDevice, vertices, indices);
            aveModel = ave::AveModel::createCubeModel(aveDevice);
            // aveModel2 = std::make_unique<AveModel>(aveDevice, vertices2, indices2);
        }

    void AveApp::generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels) {        // Check if image format supports linear blitting
            VkFormatProperties formatProperties;
            vkGetPhysicalDeviceFormatProperties(aveDevice.getPhysicalDevice(), imageFormat, &formatProperties);

            if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
                throw std::runtime_error("texture image format does not support linear blitting!");
            }

            VkCommandBuffer commandBuffer = aveDevice.beginSingleTimeCommands();

            VkImageMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.image = image;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;
            barrier.subresourceRange.levelCount = 1;

            int32_t mipWidth = texWidth;
            int32_t mipHeight = texHeight;

            for (uint32_t i = 1; i < mipLevels; i++) {
                barrier.subresourceRange.baseMipLevel = i - 1;
                barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

                vkCmdPipelineBarrier(commandBuffer,
                    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                    0, nullptr,
                    0, nullptr,
                    1, &barrier);

                VkImageBlit blit{};
                blit.srcOffsets[0] = {0, 0, 0};
                blit.srcOffsets[1] = {mipWidth, mipHeight, 1};
                blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                blit.srcSubresource.mipLevel = i - 1;
                blit.srcSubresource.baseArrayLayer = 0;
                blit.srcSubresource.layerCount = 1;
                blit.dstOffsets[0] = {0, 0, 0};
                blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
                blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                blit.dstSubresource.mipLevel = i;
                blit.dstSubresource.baseArrayLayer = 0;
                blit.dstSubresource.layerCount = 1;

                vkCmdBlitImage(commandBuffer,
                    image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    1, &blit,
                    VK_FILTER_LINEAR);

                barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                vkCmdPipelineBarrier(commandBuffer,
                    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                    0, nullptr,
                    0, nullptr,
                    1, &barrier);

                if (mipWidth > 1) mipWidth /= 2;
                if (mipHeight > 1) mipHeight /= 2;
            }

            barrier.subresourceRange.baseMipLevel = mipLevels - 1;
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            vkCmdPipelineBarrier(commandBuffer,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                0, nullptr,
                0, nullptr,
                1, &barrier);

            aveDevice.endSingleTimeCommands(commandBuffer);
        }

    // void cleanup() {
    //     vkDestroyPipelineLayout(aveDevice.device(), pipelineLayout, nullptr);
    // }

    void AveApp::recordCommandBuffer(uint32_t imageIndex) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0; // Optional
        beginInfo.pInheritanceInfo = nullptr; // Optional

        if (vkBeginCommandBuffer(commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }
        aveModel->updateUniformBuffer(aveSwapChain->getCurrentFrame(), aveSwapChain->getSwapChainExtent());
        // aveModel->updateModel();

        // Begin Drawing

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = aveSwapChain->getRenderPass();
        renderPassInfo.framebuffer = aveSwapChain->getFrameBuffer(imageIndex);
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = aveSwapChain->getSwapChainExtent();

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
        clearValues[1].depthStencil = {1.0f, 0};

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

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

        // aveModel2->bind(commandBuffers[imageIndex]);

        // vkCmdDraw(commandBuffers[imageIndex], 3, 1, 0, 0);
        // vkCmdBindDescriptorSets(commandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[aveSwapChain->getCurrentFrame()], 0, nullptr);
        // aveModel2->draw(commandBuffers[imageIndex]);


        vkCmdEndRenderPass(commandBuffers[imageIndex]);

        if (vkEndCommandBuffer(commandBuffers[imageIndex]) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }

    void AveApp::drawFrame() {
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

} // namespace ave