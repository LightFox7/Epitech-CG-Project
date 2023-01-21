#version 450 core

// Input
layout (location = 0) in flat uint instanceID;

// Output
layout (location = 0) out vec4 color;

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
	color = vec4(0.1, 1.0, 0.1, 1.0);
    if (transforms[instanceID].w == 0.0)
        color = vec4(1, 0.1, 0.1, 1.0);
}
