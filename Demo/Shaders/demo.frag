#version 450

layout (location = 0) in vec2 v_uv;

layout (location = 0) out vec4 color;

layout (push_constant) uniform PushConstants {
    float time;
} push_constants;

layout (set = 0, binding = 0) uniform Uniforms {
    float time;
} uniforms;

void main()
{
    color = vec4(v_uv, sin(uniforms.time), 1.0);
}
