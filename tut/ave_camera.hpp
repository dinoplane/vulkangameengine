#pragma once


#include "ave_device.hpp"
#include "ave_model.hpp"


namespace ave {
    class AveCamera {
        public:
            AveCamera(AveDevice& device);
            ~AveCamera();

           void createDescriptorSetLayout();

           AveDevice& aveDevice;
           VkDescriptorSetLayout descriptorSetLayout;

           VkDescriptorSetLayout* getDescriptorSetLayout() { return &descriptorSetLayout; }
    }
}