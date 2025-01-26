#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV0;
layout(location = 3) in vec2 inUV1;
layout(location = 4) in vec4 inColor0;

layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform sampler2D textures[4];

layout(set = 1, binding = 1) uniform readonly Material
{
    vec3 emissiveFactor;
    vec4 baseColorFactor; 
    float metallicFactor;
    float roughnessFactor;
    float normalScale;

    int baseColorTextureUVSet;
    int metallicRoughnessTextureUVSet;
    int NormalTextureUVSet;
    int emissiveTextureUVSet;
} material;

void main()
{
    outColor = texture(textures[0], inUV0);
}