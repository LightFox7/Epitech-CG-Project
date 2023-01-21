#version 450 core

// input vertex attributes
layout (location = 0) in vec3 position;

// Uniforms
layout(std140, binding = 1) uniform lightLayout {
    mat4 lightProjectionMatrix;
    mat4 lightViewMatrix;
    mat4 lightViewProjectionMatrix;
};

// SSBO
layout (std430, binding = 0) buffer transformLayout
{
    ivec4 indexCount;
    vec4 transforms[];
};
layout (std430, binding = 1) buffer displayLayout
{
    uint displayCount;
    uint displayIndices[];
};

void main()
{
    vec3 finalPos = position + transforms[gl_InstanceID].xyz;
    gl_Position = lightViewProjectionMatrix * vec4(finalPos, 1.0);
}