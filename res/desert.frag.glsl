#version 450 core

// Input
layout (location = 0) in smooth vec3 position;
layout (location = 1) in smooth vec3 normal;
layout (location = 2) in smooth vec2 uv;
layout (location = 3) in smooth vec4 fragPosLightSpace;
layout (location = 4) in vec3 eyeVec;

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
layout(std140, binding = 1) uniform lightLayout {
    mat4 lightProjectionMatrix;
    mat4 lightViewMatrix;
    mat4 lightViewProjectionMatrix;
};
// ShadowMap
layout (binding = 0) uniform sampler2D shadowMap;

vec3 hash3(vec2 p)
{
    vec3 q = vec3( dot(p,vec2(127.1,311.7)), 
				   dot(p,vec2(269.5,183.3)), 
				   dot(p,vec2(419.2,371.9)) );
	return fract(sin(q)*43758.5453);
}

float voronoise(vec2 p, float u, float v )
{
	float k = 1.0+63.0*pow(1.0-v,6.0);

    vec2 i = floor(p);
    vec2 f = fract(p);
    
	vec2 a = vec2(0.0,0.0);
    for( int y=-2; y<=2; y++ )
    for( int x=-2; x<=2; x++ )
    {
        vec2  g = vec2( x, y );
		vec3  o = hash3( i + g )*vec3(u,u,1.0);
		vec2  d = g - f + o.xy;
		float w = pow( 1.0-smoothstep(0.0,1.414,length(d)), k );
		a += vec2(o.z*w,w);
    }
	
    return a.x/a.y;
}

float computeShadows(float dotLightNormal)
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
    vec2 texCoord = uv - vec2(int(uv.x), int(uv.y));
    if (texCoord.x < 0) texCoord.x = 1 + texCoord.x;
    if (texCoord.y < 0) texCoord.y = 1 + texCoord.y;
    // Set material color
    vec3 materialColor = vec3(1.0, 0.95, 0.05);
    float noise = voronoise(texCoord * 64, 0.5, 0.5);
    materialColor = materialColor * (0.9 + noise * 0.3);
    // Compute diffuse lighting
    vec3 norm = normalize(normal);
    float lambertTerm = max(dot(norm, -lightDir.xyz), 0.0);
    vec3 diffLight = lambertTerm * diffuse.xyz;
    // Compute specular lighting
    vec3 r = reflect(lightDirViewSpace.xyz, norm);
    vec3 specularLight = vec3(1.0) * pow(max(dot(r, eyeVec), 0.0), 100.0);
    // Compute shadows
    float shadow = computeShadows(lambertTerm);
    vec3 finalColor = ((1.0 - shadow) * (diffLight + specularLight) + ambiant.xyz) * materialColor;
    color = vec4(finalColor, 1.0);
}

