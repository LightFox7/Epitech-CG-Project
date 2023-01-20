#version 450 core

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

// SSBO
layout (std430, binding = 0) buffer transformLayout
{
    ivec4 indexCount;
    vec4 transforms[];
};

void main()
{
	uint idx = gl_GlobalInvocationID.x;

    transforms[idx].x = idx;
    transforms[idx].y = idx;
    transforms[idx].z = idx;
}