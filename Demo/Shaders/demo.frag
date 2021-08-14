#version 450

layout (location = 0) in vec2 v_uv;

layout (location = 0) out vec4 color;

layout (push_constant) uniform PushConstants {
    float time;
} push_constants;

void main()
{
    color = vec4(v_uv, sin(push_constants.time), 1.0);
}
