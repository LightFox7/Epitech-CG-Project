#version 450 core

// Output
layout (location = 0) out vec2 fragTexCoords;

void main() 
{
    vec2 vertices[3]=vec2[3](vec2(-1,-1), vec2(3,-1), vec2(-1, 3));
    gl_Position = vec4(vertices[gl_VertexID],0,1);
    fragTexCoords = 0.5 * gl_Position.xy + vec2(0.5);
}