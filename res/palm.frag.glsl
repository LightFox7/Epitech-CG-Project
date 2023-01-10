#version 450 core

// Output
layout (location = 0) out vec4 color;

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
    color = vec4(0.6, 0.2, 0.2, 1.);
}