#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV0;
layout(location = 3) in vec2 inUV1;
layout(location = 4) in vec4 inColor0;

layout(location = 0) out vec2 outUV0;
layout(location = 1) out vec2 outUV1;
layout(location = 2) out vec4 outColor0;

layout(set = 0, binding = 0) uniform readonly ViewProj
{
    mat4 viewProj;
} viewProj;

layout(set = 0, binding = 1) uniform readonly Model
{
    mat4 model;
} model;

void main() {
    gl_Position = viewProj.viewProj * model.model * vec4(inPosition, 1.0);
    outUV0 = inUV0;
    outUV1 = inUV1;
    outColor0 = inColor0;
}