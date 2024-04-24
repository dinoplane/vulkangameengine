#version 450
// Find long lost songs... Japanese and taiwanese 1
layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragPos;
layout(location = 3) in vec3 normal;


layout(location = 0) out vec4 outColor;


layout(binding = 1) uniform sampler2D texSampler;

void main() {
    // outColor = texture(texSampler, fragTexCoord);

    vec3 lightColor = vec3(1.0, 1.0, 1.0);
    vec3 lightPos = vec3(1.0, 1.0, 1.0);


    float ambientStrength = 0.0;
    vec3 ambient = ambientStrength * lightColor;

    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(lightPos - fragPos);

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    vec3 result = (diffuse*0.7) * vec3(1.0, 0.0, 0.0);

    outColor = vec4(result, 1.0);
}