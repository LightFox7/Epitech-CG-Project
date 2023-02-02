#version 450 core

// Input
layout (location = 0) in vec2 texCoords;

// Output
layout (location = 0) out vec4 color;

// Uniforms
layout (binding = 0) uniform sampler2D hdrBuffer;
layout (binding = 1) uniform sampler2D bloomBlurBuffer;

void main()
{             
    vec3 hdrColor = texture(hdrBuffer, texCoords).rgb;
    vec3 bloomColor = texture(bloomBlurBuffer, texCoords).rgb;
    hdrColor += bloomColor; // additive blending
    const float gamma = 1.2;
    const float exposure = 2;
  
    // exposure tone mapping
    vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);
    // gamma correction 
    mapped = pow(mapped, vec3(1.0 / gamma));
  
    color = vec4(mapped, 1.0);
}