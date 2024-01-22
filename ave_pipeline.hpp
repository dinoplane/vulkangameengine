#pragma once

#include "ave_device.hpp"

#include <string>
#include <vector>

namespace ave{

    struct PipelineConfigInfo {
        VkViewport viewport;
        VkRect2D scissor;
        // VkPipelineViewportStateCreateInfo viewportInfo;
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
        VkPipelineRasterizationStateCreateInfo rasterizationInfo;
        VkPipelineMultisampleStateCreateInfo multisampleInfo;
        VkPipelineColorBlendAttachmentState colorBlendAttachment;
        VkPipelineColorBlendStateCreateInfo colorBlendInfo;
        VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
        VkPipelineLayout pipelineLayout = nullptr;
        VkRenderPass renderPass = nullptr;
        uint32_t subpass = 0;
    };

    class AvePipeline {
        public:
            AvePipeline(AveDevice &device, const std::string& vertFilePath, const std::string& fragFilePath, const PipelineConfigInfo &configInfo);
            ~AvePipeline();

            AvePipeline(const AvePipeline&) = delete;
            void operator=(const AvePipeline&) = delete;

            void bind(VkCommandBuffer commandBuffer);

            static PipelineConfigInfo defaultPipelineConfigInfo(uint32_t width, uint32_t height);

        private:
            static std::vector<char> readFile(const std::string& filepath);

            void createGraphicsPipeline(const std::string& vertFilePath, const std::string& fragFilePath, const PipelineConfigInfo& configInfo);

            void createShaderModule(const std::vector<char>& shaderCode, VkShaderModule* shaderModule);

            AveDevice& aveDevice; // potentially memory unsafe, but the device will outlive the pipeline so no risk of dangling ptr
            VkPipeline graphicsPipeline;
            VkShaderModule vertShaderModule;
            VkShaderModule fragShaderModule; // typedef ptrs,
    };
}