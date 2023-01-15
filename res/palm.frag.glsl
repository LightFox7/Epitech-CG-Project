#version 450 core

// input
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

// Output
layout (location = 0) out vec4 color;

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
    vec3 materialColor = abs(normal);
    vec3 norm = gl_FrontFacing ? normalize(normal) : normalize(-normal);
    float lambertTerm = max(dot(norm, -lightDirViewSpace.xyz), 0.0);
    vec3 diffLight = lambertTerm * diffuse.xyz;
    vec3 finalColor = (ambiant.xyz + diffLight) * materialColor;
    color = vec4(finalColor, 1.);
}