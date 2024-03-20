#version 450

// layout (location = 0) in vec3 fragColor;


layout (location = 0) out vec4 outColor;

layout (push_constant) uniform Push {
    vec3 color;
    mat2 model;
} push;

void main(){
    outColor = vec4(push.color, 1.0);
    // gl_FragColor = outColor;
}
