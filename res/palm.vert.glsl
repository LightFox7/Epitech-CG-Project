#version 450 core

// input vertex attributes
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

layout (location = 0) out vec3 fragPosition;
layout (location = 1) out vec3 fragNormal;

// Uniforms
layout(std140, binding = 0) uniform uniformLayout
{
    mat4 viewProjectionMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;
    vec4 lightPos;
    vec4 lightDir;
    vec4 lightDirViewSpace;
    vec4 ambiant;
    vec4 diffuse;
};

// SSBO
layout (std430, binding = 1) buffer transformLayout
{
    vec4 transforms[];
};

void main()
{
    vec3 finalPos = position + transforms[gl_InstanceID].xyz;
    gl_Position = viewProjectionMatrix * vec4(finalPos, 1.0);

    fragPosition = finalPos;
    fragNormal = normal;
}