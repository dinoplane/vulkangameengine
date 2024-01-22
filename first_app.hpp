#pragma once

#include "ave_device.hpp"
#include "ave_pipeline.hpp"
#include "ave_swap_chain.hpp"
#include "ave_window.hpp"


#include <memory>
#include <vector>


namespace ave {
    class FirstApp {

        public:
            static constexpr int WIDTH = 800;
            static constexpr int HEIGHT = 600;

            FirstApp();
            ~FirstApp();

            FirstApp(const FirstApp&) = delete;
            FirstApp& operator=(const FirstApp&) = delete;


            void run();

        private:
            void createPipelineLayout();
            void createPipeline();
            void createCommandBuffers();
            void drawFrame();

            AveWindow aveWindow{WIDTH, HEIGHT, "Hello Vulkan"};
            AveDevice aveDevice{aveWindow};
            AveSwapChain aveSwapChain{aveDevice, aveWindow.getExtent()};
            std::unique_ptr<AvePipeline> avePipeline;

            /*{aveDevice,
            "shaders/simple_shader.vert.spv",
            "shaders/simple_shader.frag.spv",

            AvePipeline::defaultPipelineConfigInfo(WIDTH, HEIGHT)};
*/

            VkPipelineLayout pipelineLayout;
            std::vector<VkCommandBuffer> commandBuffers;
    };
}