#version 450

layout(location = 0) in vec2 inUV0;
layout(location = 1) in vec2 inUV1;
layout(location = 2) in vec4 inColor0;

layout(location = 0) out vec4 outColor;

void main()
{
    outColor = inColor0;
}