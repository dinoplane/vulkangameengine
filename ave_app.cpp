#include "ave_app.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>


#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobjloader/tiny_obj_loader.h>

namespace ave{
void AveApp::createTextureImage(){
    int texWidth, texHeight, texChannels;
        // stbi_uc* pixels = stbi_load("textures/texture.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        stbi_uc* pixels = stbi_load(TEXTURE_PATH.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

        VkDeviceSize imageSize = texWidth * texHeight * 4;

        if (!pixels) {
            throw std::runtime_error("failed to load texture image!");
        }

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;

        aveDevice.createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* data;
        vkMapMemory(aveDevice.device(), stagingBufferMemory, 0, imageSize, 0, &data);
            memcpy(data, pixels, static_cast<size_t>(imageSize));
        vkUnmapMemory(aveDevice.device(), stagingBufferMemory);

        stbi_image_free(pixels);
        aveDevice.createImage(texWidth, texHeight, mipLevels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

        aveDevice.transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);
        aveDevice.copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 1);
        // aveDevice.transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);



        vkDestroyBuffer(aveDevice.device(), stagingBuffer, nullptr);
        vkFreeMemory(aveDevice.device(), stagingBufferMemory, nullptr);

        generateMipmaps(textureImage, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, mipLevels);

}

void AveApp::loadModels(){
        // const std::vector<Vertex> vertices = {
        //     {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        //     {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
        //     {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
        // };



        std::vector<Vertex> vertices;
        std::vector<Vertex> vertices2;

        // FirstApp::triforce(vertices,
        //     {{0.0f, -1.0f}, {1.0, 0.0, 1.0}},
        //     {{1.0f, 1.0f}, {1.0, 1.0, 0.0}},
        //     {{-1.0f, 1.0f}, {0.0, 1.0, 1.0}},
        //     7);
        vertices2.insert(vertices2.end(),{
            {{-0.5f, -0.5f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
            {{0.5f, -0.5f, 1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
            {{0.5f, 0.5f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
            {{-0.5f, 0.5f, 1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

            {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
            {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
            {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
            {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
        });

        std::vector<uint32_t> indices;// = {
        //     0, 1, 2, 2, 3, 0,
        //     4, 5, 6, 6, 7, 4
        // };


        std::vector<uint32_t> indices2 = {
            0, 1, 2,
            4, 5, 6
        };


        // size_t NUM_FACE_EDGES = 36;
        // size_t NUM_HEIGHT = 5;
        // float delta_theta = 2*M_PI / NUM_FACE_EDGES;


        // for (size_t i = 0; i < NUM_FACE_EDGES; i++){

        //     Vertex vertex{};

        //     vertex.pos = {
        //         0.5f * cos(i * delta_theta),
        //         0.5f * sin(i * delta_theta),
        //         0.0f
        //     };

        //     vertex.texCoord = {
        //         0.5f + 0.5f * cos(i * delta_theta),
        //         0.5f + 0.5f * sin(i * delta_theta)
        //     };

        //     vertex.color = {1.0f, 1.0f, 1.0f};

        //     vertices.push_back(vertex);
        // }

        // Vertex vertex{};
        // vertices.push_back(vertex);

        // for (size_t i = 0; i < NUM_FACE_EDGES - 1; i++){
        //     indices.push_back(NUM_FACE_EDGES - 1);
        //     indices.push_back(i);
        //     indices.push_back((i+1) % (NUM_FACE_EDGES - 1));
        // }

        size_t RESOLUTION = 36;
        size_t NUM_HEIGHT = 5;
        float start_height = -0.3f;
        float delta_z = 1.0f / static_cast<float>(RESOLUTION);
        float delta_theta = 2*M_PI / static_cast<float>(RESOLUTION);

        for (size_t j = 0; j < RESOLUTION; j++) {
            float z =  (j * delta_z);
            float r =  0.5*sin(M_PI * j / RESOLUTION);
            for (size_t i = 0; i < RESOLUTION; i++){

                Vertex vertex{};


                vertex.pos = {
                    r * cos(M_PI * 2 * i / RESOLUTION ),
                    r * sin(M_PI * 2 * i / RESOLUTION ),
                    start_height + 0.5*cos(M_PI * j / RESOLUTION)
                };

                vertex.texCoord = {
                    0.5f + 0.5f * cos(i * delta_theta),
                    0.5f + 0.5f * sin(i * delta_theta)
                };

                vertex.color = {1.0f, 1.0f, 1.0f};

                vertices.push_back(vertex);

            }
        }

        for (size_t j = 0; j < RESOLUTION - 1; j++) {
            for (size_t i = 0; i < RESOLUTION; i++){
                indices.push_back(j*RESOLUTION + i);
                indices.push_back(j*RESOLUTION + ((i+1) % RESOLUTION));
                indices.push_back((j+1)*RESOLUTION + i);

                indices.push_back((j+1)*RESOLUTION + i);
                indices.push_back(j*RESOLUTION + ((i+1) % RESOLUTION));
                indices.push_back((j+1)*RESOLUTION + ((i+1) % RESOLUTION));
            }
        }

        // tinyobj::attrib_t attrib;
        // std::vector<tinyobj::shape_t> shapes;
        // std::vector<tinyobj::material_t> materials;
        // std::string warn, err;

        // if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, MODEL_PATH.c_str())) {
        //     throw std::runtime_error(warn + err);
        // }


        // std::unordered_map<ave::Vertex, uint32_t, ave::Vertex_hash> uniqueVertices;


        // for (const auto& shape : shapes) {
        //     for (const auto& index : shape.mesh.indices) {
        //         Vertex vertex{};

        //         vertex.pos = {
        //             attrib.vertices[3 * index.vertex_index + 0],
        //             attrib.vertices[3 * index.vertex_index + 1],
        //             attrib.vertices[3 * index.vertex_index + 2]
        //         };

        //         vertex.texCoord = {
        //             attrib.texcoords[2 * index.texcoord_index + 0],
        //             1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
        //         };

        //         vertex.color = {1.0f, 1.0f, 1.0f};

        //         if (uniqueVertices.count(vertex) == 0) {
        //             uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
        //             vertices.push_back(vertex);
        //         }

        //         indices.push_back(uniqueVertices[vertex]);
        //     }
        // }

        aveModel = std::make_unique<AveModel>(aveDevice, vertices, indices);
        aveModel2 = std::make_unique<AveModel>(aveDevice, vertices2, indices2);
    }



} // namespace ave