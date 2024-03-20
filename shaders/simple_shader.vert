#version 450

layout (location = 0) in vec2 position;
layout (location = 1) in vec3 color;

// layout (location = 0) out vec3 fragColor;

layout (push_constant) uniform Push {
    vec3 color;
    mat2 model;
} push;

void main(){
    // fragColor = push.color;
    gl_Position = vec4(push.model*position, 0.0f, 1.0);
}
