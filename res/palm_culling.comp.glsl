#version 450 core

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

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
layout(std140, binding = 2) uniform frustumLayout
{
    vec4 topFace;
    vec4 bottomFace;
    vec4 rightFace;
    vec4 leftFace;
    vec4 farFace;
    vec4 nearFace;
};
// SSBO
layout (std430, binding = 0) buffer transformLayout
{
    ivec4 indexCount;
    vec4 transforms[];
};

float getSignedDistanceToPlane(vec4 plane, vec3 point)
{
    vec3 normal = plane.xyz;
    float dist = plane.w;
    return dot(normal, point) - dist;
};

bool isOnOrForwardPlane(vec4 plane, vec3 center, float radius)
{
    return getSignedDistanceToPlane(plane, center) > -radius;
};

bool isOnFrustum(vec3 center, float radius)
{
//         isOnOrForwardPlane(farFace, center, radius) &&
    return (isOnOrForwardPlane(leftFace, center, radius) &&
        isOnOrForwardPlane(rightFace, center, radius) &&
        isOnOrForwardPlane(nearFace, center, radius) &&
        isOnOrForwardPlane(topFace, center, radius) &&
        isOnOrForwardPlane(bottomFace, center, radius));
};

void main()
{
	uint idx = gl_GlobalInvocationID.x;

    float radius = 2.5;
    vec3 center = transforms[idx].xyz + vec3(0, radius, 0);
    bool onFrustum = isOnFrustum(center, radius);
    transforms[idx].w = onFrustum ? 1.0 : 0.0;
}