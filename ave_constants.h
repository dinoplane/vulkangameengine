#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>
#include <cstring>
#include <vector>

namespace ave {
    static constexpr int MAX_FRAMES_IN_FLIGHT = 2;
    const std::string MODEL_PATH = "models/viking_room.obj";
    const std::string TEXTURE_PATH = "textures/viking_room.png";
}


