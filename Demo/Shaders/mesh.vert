#version 450

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;
layout (location = 0) out vec2 v_uv;

void main()
{
    v_uv = uv;
    gl_Position = vec4(position, 1);
}
