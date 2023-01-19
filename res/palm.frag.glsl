#version 450 core

// input
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;

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
layout(std140, binding = 1) uniform lightLayout {
    mat4 lightProjectionMatrix;
    mat4 lightViewMatrix;
    mat4 lightViewProjectionMatrix;
};

// SSBO
layout (std430, binding = 1) buffer transformLayout
{
    vec4 transforms[];
};

// ShadowMap
layout (binding = 0) uniform sampler2D shadowMap;

float computeShadows(vec4 fragPosLightSpace, float dotLightNormal)
{
    // Convert from [-1, 1] to [0, 1] range
    vec3 projCoords = (fragPosLightSpace.xyz / fragPosLightSpace.w) * 0.5 + 0.5;
    float depth = texture(shadowMap, projCoords.xy).r;

    float bias = 0.003;
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += projCoords.z - bias > pcfDepth ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
    if (projCoords.z > 1.0)
        shadow = 0.0;
    return shadow;
}


void main()
{
    vec4 fragPosLightSpace = lightViewProjectionMatrix * vec4(position, 1.0);
    // Set material color function of uvs (if both are 0: those are leaves, else, it is the trunk)
    vec3 materialColor = vec3(0.2, 0.9, 0.2);
    if (uv.x != 0 || uv.y != 0) materialColor = vec3(0.6, 0.4, 0.3); 
    vec3 norm = gl_FrontFacing ? normalize(normal) : normalize(-normal);
    float lambertTerm = max(dot(norm, -lightDir.xyz), 0.0);
    vec3 diffLight = lambertTerm * diffuse.xyz;
    // Compute shadows
    float shadow = computeShadows(fragPosLightSpace, lambertTerm);
    vec3 finalColor = ((1.0 - shadow) * (diffLight) + ambiant.xyz) * materialColor;
    color = vec4(finalColor, 1.);
}