#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>

namespace ave {
    class AveWindow {
    public:
        AveWindow(int w, int h, std::string name);
        ~AveWindow();


        // Delete copy constructors
        AveWindow(const AveWindow&) = delete;
        AveWindow &operator=(const AveWindow&) = delete;

        bool shouldClose() {return glfwWindowShouldClose(window);};
        void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface);

        VkExtent2D getExtent() { return {static_cast<u_int32_t>(width), static_cast<u_int32_t>(height)}; };

    private:
        void initWindow();
        const int width;
        const int height;

        std::string windowName;
        GLFWwindow * window;
    };
}