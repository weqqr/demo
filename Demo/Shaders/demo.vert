#version 450

layout (location = 0) out vec2 v_uv;

void main()
{
    vec2 position = vec2(
        ((gl_VertexIndex & 1) << 2) - 1,
        ((gl_VertexIndex & 2) << 1) - 1
    );

    v_uv = (position + 1) / 2;
    gl_Position = vec4(position, 0, 1);
}
