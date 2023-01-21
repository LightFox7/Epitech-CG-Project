#version 450 core

// input vertex attributes
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;

layout (location = 0) out flat uint instanceID;

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

// SSBO
layout (std430, binding = 0) buffer transformLayout
{
    ivec4 indexCount;
    vec4 transforms[];
};

void main()
{
	vec3 finalPos = position + transforms[gl_InstanceID].xyz + vec3(0.0, 2.5, 0.0);
    gl_Position = viewProjectionMatrix * vec4(finalPos, 1.0);
    instanceID = gl_InstanceID;
}
