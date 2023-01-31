#version 450 core

// Input
layout (location = 0) in vec2 texCoords;

// Output
layout (location = 0) out vec4 color;

// Uniforms
layout (binding = 0) uniform sampler2D hdrBuffer;

void main()
{             
    vec3 hdrColor = texture(hdrBuffer, texCoords).rgb;
    const float gamma = 1.1;
    const float exposure = 2.0;
  
    // exposure tone mapping
    vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);
    // gamma correction 
    mapped = pow(mapped, vec3(1.0 / gamma));
  
    color = vec4(mapped, 1.0);
}