#version 450 core

// input vertex attributes
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

// Uniforms
layout(std140, binding = 0) uniform Matrix
{
    mat4 modelViewProjection;
};

// SSBO
layout (std430, binding = 1) buffer transformLayout
{
    vec4 transforms[];
};

void main()
{
    vec3 finalPos = vec3(position + transforms[gl_InstanceID].xyz);
    gl_Position = modelViewProjection * vec4(finalPos, 1.);
}