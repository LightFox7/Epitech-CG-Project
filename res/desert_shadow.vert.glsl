#version 450 core

// input vertex attributes
layout (location = 0) in vec3 position;

// Uniforms
layout(std140, binding = 1) uniform lightLayout {
    mat4 lightProjectionMatrix;
    mat4 lightViewMatrix;
    mat4 lightViewProjectionMatrix;
};

void main()
{
    gl_Position = lightViewProjectionMatrix * vec4(position, 1.0);
}