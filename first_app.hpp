#pragma once

#include "ave_device.hpp"
#include "ave_pipeline.hpp"
#include "ave_swap_chain.hpp"
#include "ave_window.hpp"
#include "ave_model.hpp"

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

            // static void triforce(std::vector<AveModel::Vertex> &v, float n);
            static void triforce(std::vector<AveModel::Vertex> &v, AveModel::Vertex v1, AveModel::Vertex v2, AveModel::Vertex v3, float n);

            void run();

        private:
            void loadModels();
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

            std::unique_ptr<AveModel> aveModel;
    };
}