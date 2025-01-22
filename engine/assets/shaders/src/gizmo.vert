#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inColor;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform readonly ViewProj
{
    mat4 viewProj;
} viewProj;

void main() {
    gl_Position = viewProj.viewProj * vec4(inPosition, 1.0);
    outColor = inColor;
}