#version 450 core

// input vertex attributes
layout (location = 0) in vec3 position;

// Uniforms
layout (std140, binding = 0) uniform uniformLayout {
    mat4 viewProjectionMatrix;
};

void main()
{
    vec3 finalPos = vec3(position);
    gl_Position = viewProjectionMatrix * vec4(finalPos, 1.0f);
}