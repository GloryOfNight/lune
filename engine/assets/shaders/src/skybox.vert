#version 450

layout(location = 0) in vec3 inPosition;

layout(location = 0) out vec3 outUVW;

layout(set = 0, binding = 0) uniform readonly View
{
    mat4 view;
} view;

layout(set = 0, binding = 1) uniform readonly Proj 
{
    mat4 proj;
} proj;

void main() {
    gl_Position = proj.proj * mat4(mat3(view.view)) * vec4(inPosition, 0.1);
    outUVW = inPosition;
}