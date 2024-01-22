#version 450

layout (location = 1)in vec2 uv;
layout (location = 0) out vec4 outColor;

void main(){
    outColor = vec4(uv, 1.0, 1.0);
    // gl_FragColor = outColor;
}
