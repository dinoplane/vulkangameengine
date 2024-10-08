#pragma once

// vulkan headers
#include <vulkan/vulkan.h>

#include "ave_constants.h"
#include "ave_device.hpp"

// std lib headers
#include <array>
#include <limits>
#include <cstdlib>
#include <set>
#include <memory>
#include <algorithm>

namespace ave {
class AveSwapChain {
 public:


  AveSwapChain(AveDevice &deviceRef, VkExtent2D windowExtent);
  AveSwapChain(AveDevice &deviceRef, VkExtent2D windowExtent, std::shared_ptr<AveSwapChain> previous);
  ~AveSwapChain();

  AveSwapChain(const AveSwapChain &) = delete;
  AveSwapChain& operator=(const AveSwapChain &) = delete;

  VkFramebuffer getFrameBuffer(int index) { return swapChainFramebuffers[index]; }
  VkRenderPass getRenderPass() { return renderPass; }
  VkImageView getImageView(int index) { return swapChainImageViews[index]; }
  size_t imageCount() { return swapChainImages.size(); }
  VkFormat getSwapChainImageFormat() { return swapChainImageFormat; }
  VkExtent2D getSwapChainExtent() { return swapChainExtent; }
  size_t getCurrentFrame() { return currentFrame; }


  uint32_t width() { return swapChainExtent.width; }
  uint32_t height() { return swapChainExtent.height; }

  float extentAspectRatio() {
    return static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height);
  }
  VkFormat findDepthFormat() {
    return aveDevice.findDepthFormat();
  }

  VkResult acquireNextImage(uint32_t *imageIndex);
  VkResult submitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex);

  VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
  void createImageViews();
 private:
  void init();
  void createSwapChain();
  void createColorResources();
  void createDepthResources();
  void createRenderPass();
  void createFramebuffers();
  void createSyncObjects();

  // Helper functions
  VkSurfaceFormatKHR chooseSwapSurfaceFormat(
      const std::vector<VkSurfaceFormatKHR> &availableFormats);
  VkPresentModeKHR chooseSwapPresentMode(
      const std::vector<VkPresentModeKHR> &availablePresentModes);
  VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

  VkFormat swapChainImageFormat;
  VkExtent2D swapChainExtent;

  std::vector<VkFramebuffer> swapChainFramebuffers;
  VkRenderPass renderPass;

  std::vector<VkImage> depthImages;
  std::vector<VkDeviceMemory> depthImageMemorys;
  std::vector<VkImageView> depthImageViews;
  std::vector<VkImage> swapChainImages;
  std::vector<VkImageView> swapChainImageViews;


  VkImage colorImage;
  VkDeviceMemory colorImageMemory;
  VkImageView colorImageView;

  AveDevice &aveDevice;
  VkExtent2D windowExtent;

  VkSwapchainKHR swapChain;
  std::shared_ptr<AveSwapChain> oldSwapchain;

  std::vector<VkSemaphore> imageAvailableSemaphores;
  std::vector<VkSemaphore> renderFinishedSemaphores;
  std::vector<VkFence> inFlightFences;
  std::vector<VkFence> imagesInFlight;
  size_t currentFrame = 0;
};

}  // namespace ave
