#version 450 core

// Output
layout (location = 0) out vec4 color;

// Uniforms
layout (std140, binding = 0) uniform uniformLayout {
    mat4 viewProjectionMatrix;
};

void main()
{
    color = vec4(0.7f, 0.7f, 0.1f, 1.0f);
}