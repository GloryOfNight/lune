#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV0;
layout(location = 3) in vec2 inUV1;
layout(location = 4) in vec4 inColor0;

layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform sampler2D textures[5]; // 0 - base, 1 - metallicRoughness, 2 - normal, 3 - occlusion, 4 - emissive

layout(set = 1, binding = 1) uniform readonly Material
{
    vec3 emissiveFactor;
    vec4 baseColorFactor;
    float metallicFactor;
    float roughnessFactor;
    float normalScale;

    // UV sets either 0-1 or -1 (no texture)
    int baseColorTextureUVSet;
    int metallicRoughnessTextureUVSet;
    int normalTextureUVSet;
    int occlusionTextureUVSet;
    int emissiveTextureUVSet;
} material;

void main()
{
    // Base Color
    vec4 baseColor = material.baseColorFactor;
    if (material.baseColorTextureUVSet >= 0)
    {
        vec2 baseUV = (material.baseColorTextureUVSet == 0) ? inUV0 : inUV1;
        baseColor *= texture(textures[0], baseUV);
    }

    // Metallic & Roughness
    vec2 metallicRoughnessUV = (material.metallicRoughnessTextureUVSet == 0) ? inUV0 : inUV1;
    vec2 metallicRoughnessValues = vec2(material.metallicFactor, material.roughnessFactor);
    if (material.metallicRoughnessTextureUVSet >= 0)
    {
        vec4 metallicRoughnessTex = texture(textures[1], metallicRoughnessUV);
        metallicRoughnessValues = metallicRoughnessTex.rg;
    }
    float metallic = metallicRoughnessValues.x;
    float roughness = metallicRoughnessValues.y;

    // Normal Map
    vec3 normal = normalize(inNormal);
    if (material.normalTextureUVSet >= 0)
    {
        vec2 normalUV = (material.normalTextureUVSet == 0) ? inUV0 : inUV1;
        vec3 normalTex = texture(textures[2], normalUV).rgb;
        normalTex = normalTex * 2.0 - 1.0; // Convert to [-1,1] range
        normal = normalize(normal + material.normalScale * normalTex);
    }

    // Occlusion (Ambient Occlusion)
    float occlusion = 1.0;
    if (material.occlusionTextureUVSet >= 0)
    {
        vec2 occlusionUV = (material.occlusionTextureUVSet == 0) ? inUV0 : inUV1;
        occlusion = texture(textures[3], occlusionUV).r;
    }

    // Emissive Color
    vec3 emissive = material.emissiveFactor;
    if (material.emissiveTextureUVSet >= 0)
    {
        vec2 emissiveUV = (material.emissiveTextureUVSet == 0) ? inUV0 : inUV1;
        emissive *= texture(textures[4], emissiveUV).rgb;
    }

    // Final Color (No lighting, just textures & factors)
    vec3 finalColor = baseColor.rgb * occlusion + emissive;

    outColor = vec4(finalColor, baseColor.a);
}
