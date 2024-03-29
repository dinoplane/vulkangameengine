
#include "ave_window.hpp"
#include <stdexcept>


namespace ave {

    AveWindow::AveWindow(int w, int h, std::string name) : width(w), height(h), windowName(name){
        initWindow();
    };


    void AveWindow::initWindow(){
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // do not init OpenGL context
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE); // prevent resizing

        window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, frameBufferResizeCallback);
    };

    AveWindow::~AveWindow(){
        glfwDestroyWindow(window);
        glfwTerminate();
    };


    void AveWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR *surface){
        if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS){
            throw std::runtime_error("Failed to create window surface");
        }
    }


    void AveWindow::frameBufferResizeCallback(GLFWwindow *window, int width, int height){
        auto aveWindow = reinterpret_cast<AveWindow *>(glfwGetWindowUserPointer(window));
        aveWindow->width = width;
        aveWindow->height = height;
    }
}