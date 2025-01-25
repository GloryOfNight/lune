#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;

layout(location = 0) out vec4 outColor;

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
    outColor = vec4(inPosition, 1.0);
}