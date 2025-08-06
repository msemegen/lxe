#version 450

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

layout(set = 0, binding = 0) uniform MVP {
    mat4 mvp;
} ubo;

void main() {
    vec4 pos = vec4(inPosition, 0.0, 1.0);
    gl_Position = ubo.mvp * pos;
    fragColor = inColor;
}
