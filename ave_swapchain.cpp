#include "ave_swapchain.hpp"

namespace ave {
    AveSwapChain::AveSwapChain(AveDevice &deviceRef, VkExtent2D extent)
        : aveDevice{deviceRef}, windowExtent{extent} {
    init();
    }

    AveSwapChain::AveSwapChain(AveDevice &deviceRef, VkExtent2D extent, std::shared_ptr<AveSwapChain> previous)
        : aveDevice{deviceRef}, windowExtent{extent} , oldSwapchain{previous}{
    init();

    oldSwapchain = nullptr;
    }

    AveSwapChain::~AveSwapChain() {
        for (auto imageView : swapChainImageViews) {
            vkDestroyImageView(aveDevice.device(), imageView, nullptr);
        }
        swapChainImageViews.clear();

        if (swapChain != nullptr) {
            vkDestroySwapchainKHR(aveDevice.device(), swapChain, nullptr);
            swapChain = nullptr;
        }

        for (int i = 0; i < depthImages.size(); i++) {
            vkDestroyImageView(aveDevice.device(), depthImageViews[i], nullptr);
            vkDestroyImage(aveDevice.device(), depthImages[i], nullptr);
            vkFreeMemory(aveDevice.device(), depthImageMemorys[i], nullptr);
        }

        for (auto framebuffer : swapChainFramebuffers) {
            vkDestroyFramebuffer(aveDevice.device(), framebuffer, nullptr);
        }

        vkDestroyRenderPass(aveDevice.device(), renderPass, nullptr);

        // cleanup synchronization objects
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(aveDevice.device(), renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(aveDevice.device(), imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(aveDevice.device(), inFlightFences[i], nullptr);
        }
    }

    VkResult AveSwapChain::acquireNextImage(uint32_t *imageIndex) {
    vkWaitForFences(
        aveDevice.device(),
        1,
        &inFlightFences[currentFrame],
        VK_TRUE,
        std::numeric_limits<uint64_t>::max());

    VkResult result = vkAcquireNextImageKHR(
        aveDevice.device(),
        swapChain,
        std::numeric_limits<uint64_t>::max(),
        imageAvailableSemaphores[currentFrame],  // must be a not signaled semaphore
        VK_NULL_HANDLE,
        imageIndex);

    return result;
    }

    VkResult AveSwapChain::submitCommandBuffers(
        const VkCommandBuffer *buffers, uint32_t *imageIndex) {
    if (imagesInFlight[*imageIndex] != VK_NULL_HANDLE) {
        vkWaitForFences(aveDevice.device(), 1, &imagesInFlight[*imageIndex], VK_TRUE, UINT64_MAX);
    }
    imagesInFlight[*imageIndex] = inFlightFences[currentFrame];

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = buffers;

    VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkResetFences(aveDevice.device(), 1, &inFlightFences[currentFrame]);
    if (vkQueueSubmit(aveDevice.graphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame]) !=
        VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = imageIndex;

    auto result = vkQueuePresentKHR(aveDevice.presentQueue(), &presentInfo);

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

    return result;
    }




    void AveSwapChain::init(){
    createSwapChain();
    createImageViews();
    createRenderPass();
    // createDepthResources();
    createFramebuffers();
    createSyncObjects();
    }

    void AveSwapChain::createSwapChain() {
        // SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);
        SwapChainSupportDetails swapChainSupport = aveDevice.getSwapChainSupport();


        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

        swapChainImageFormat = surfaceFormat.format;
        swapChainExtent = extent;

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = aveDevice.surface();

        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        // QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
        QueueFamilyIndices indices = aveDevice.findPhysicalQueueFamilies();

        uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

        if (indices.graphicsFamily != indices.presentFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        } else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0; // Optional
            createInfo.pQueueFamilyIndices = nullptr; // Optional
        }

        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;

        createInfo.oldSwapchain = VK_NULL_HANDLE;

        if (vkCreateSwapchainKHR(aveDevice.device(), &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
            throw std::runtime_error("failed to create swap chain!");
        }

        vkGetSwapchainImagesKHR(aveDevice.device(), swapChain, &imageCount, nullptr);
        swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(aveDevice.device(), swapChain, &imageCount, swapChainImages.data());


    }

    void AveSwapChain::createImageViews() {
        swapChainImageViews.resize(swapChainImages.size());
        for (size_t i = 0; i < swapChainImages.size(); i++) {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = swapChainImages[i];

            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = swapChainImageFormat;

            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(aveDevice.device(), &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create image views!");
            }

        }
    }

    void AveSwapChain::createFramebuffers() {
        swapChainFramebuffers.resize(swapChainImageViews.size());
        for (size_t i = 0; i < swapChainImageViews.size(); i++) {
            VkImageView attachments[] = {
                swapChainImageViews[i]
            };

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = renderPass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = attachments;
            framebufferInfo.width = swapChainExtent.width;
            framebufferInfo.height = swapChainExtent.height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(aveDevice.device(), &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create framebuffer!");
            }
        }
    }

    void AveSwapChain::createRenderPass() {

        // Attachments (an image on a swap chain)
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = swapChainImageFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;


        // Subpasses (subsequent rendering operations that depend on the contens of frame buffers in previous passes (i.e. a sequence of post processing effects))
        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;

        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;

        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        if (vkCreateRenderPass(aveDevice.device(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
            throw std::runtime_error("failed to create render pass!");
        }

    }



    void AveSwapChain::createSyncObjects() {
        imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
        imagesInFlight.resize(imageCount(), VK_NULL_HANDLE);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            if (vkCreateSemaphore(aveDevice.device(), &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(aveDevice.device(), &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(aveDevice.device(), &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {

                throw std::runtime_error("failed to create synchronization objects for a frame!");
            }
        }
    }

        // The format of the image (what each pixel looks like)
    VkSurfaceFormatKHR AveSwapChain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat; // http://stackoverflow.com/questions/12524623/
            }
        }

        return availableFormats[0];
    }

    // Conditions for showing images to the screen
    // IMMEDIATE: images submitted by app are transferred to screen right away
    // FIFO: images are queued and displayed in order. If queue is full, app has to wait (V-sync). vertical blank: when display is refreshed
    // FIFO_RELAXED: same as FIFO, but display immediately if queue empty at last vertical blank and app is late
    // MAILBOX: triple buffering. Images are queued and displayed in order. If queue is full, the last image is replaced with the new one
    VkPresentModeKHR AveSwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
        for (const auto& availablePresentMode : availablePresentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return availablePresentMode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    // Swap extent is the resolution of the swap chain images. Almost always equal to the resolution of the window in pixels

    VkExtent2D AveSwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        } else {
            // int width, height;
            // aveWindow.getFramebufferSize(&width, &height);

            // VkExtent2D actualExtent = {
            //     static_cast<uint32_t>(width),
            //     static_cast<uint32_t>(height)
            // };

            // actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            // actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            // return actualExtent;
            VkExtent2D actualExtent = windowExtent;
            actualExtent.width = std::max(
                capabilities.minImageExtent.width,
                std::min(capabilities.maxImageExtent.width, actualExtent.width));
            actualExtent.height = std::max(
                capabilities.minImageExtent.height,
                std::min(capabilities.maxImageExtent.height, actualExtent.height));

            return actualExtent;

        }
    }




}
