#version 450 core

// Input
layout (location = 0) in vec2 texCoords;

// Output
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

// Uniforms
layout (binding = 0) uniform sampler2D hdrBuffer;

void main()
{            
    vec4 FragColor = vec4(texture(hdrBuffer, texCoords).rgb, 1.0);
    // check whether fragment output is higher than threshold, if so output as brightness color
    float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if (brightness > 1.0)
        BrightColor = vec4(FragColor.rgb, 1.0);
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}