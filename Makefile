GLSLC = glslc

CFLAGS = -std=c++17 -O2
LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

vertSources = $(shell find ./shaders -type f -name "*.vert")
vertObjFiles = $(patsubst %.vert, %.vert.spv, $(vertSources))
fragSources = $(shell find ./shaders -type f -name "*.frag")
fragObjFiles = $(patsubst %.frag, %.frag.spv, $(fragSources))

TARGET = VulkanGameEngine

$(TARGET): $(vertObjFiles) $(fragObjFiles)

# $(TARGET): *.cpp *.hpp
# 	g++ $(CFLAGS) -o $(TARGET) *.cpp $(LDFLAGS)

$(TARGET): main.cpp ave_window.cpp ave_window.hpp ave_device.cpp ave_device.hpp
	g++ $(CFLAGS) -o $(TARGET) main.cpp ave_window.cpp ave_window.hpp ave_device.cpp ave_device.hpp $(LDFLAGS)

%.spv: %
	${GLSLC} $< -o $@

.PHONY: test clean

test: VulkanGameEngine
	./VulkanGameEngine

clean:
	rm -f VulkanGameEngine
	rm -f ./shaders/*.spv