#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inColor;
layout(location = 3) in vec2 inTexCoord;


layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 fragPos;
layout(location = 3) out vec3 normal;


void main() {
    fragPos = vec3(ubo.model * vec4(inPosition, 1.0));
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);

    bvec3 bozo = bvec3(inNormal.x != 0.0, inNormal.y != 0.0, inNormal.z != 0.0);

    fragColor = clamp((inNormal + vec3(bozo)*3.0)/4.0, vec3(0.0), vec3(1.0));
    fragTexCoord = inTexCoord;
    normal =  vec3(ubo.model * vec4(inNormal, 0.0));
}