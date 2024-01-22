#version 450

vec2 positions[3] = vec2[](
    vec2(0.0, -0.5),
    vec2(-0.5, 0.5),
    vec2(0.5, 0.5)
);

layout (location = 1) out vec2 uv;
void main(){
    uv = positions[gl_VertexIndex % 3] + vec2(0.5);
    gl_Position = vec4(positions[gl_VertexIndex % 3], 0.0, 1.0);
}
