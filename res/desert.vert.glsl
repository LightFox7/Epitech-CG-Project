#version 450 core

// Input vertex attributes
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;

// Output
layout (location = 0) out vec3 fragPos;
layout (location = 1) out vec3 fragNormal;
layout (location = 2) out vec2 fragUv;

// Uniforms
layout(std140, binding = 0) uniform uniformLayout
{
    mat4 viewProjectionMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;
    vec4 lightDir;
    vec4 lightDirViewSpace;
    vec4 ambiant;
    vec4 diffuse;
};
// Uniforms
layout(std140, binding = 1) uniform lightLayout {
    mat4 lightProjectionMatrix;
    mat4 lightViewMatrix;
    mat4 lightViewProjectionMatrix;
};

void main()
{
    vec3 finalPos = vec3(position);
    gl_Position = viewProjectionMatrix * vec4(finalPos, 1.0f);
    fragPos = finalPos;
    fragNormal = normal;
    fragUv = uv;
}