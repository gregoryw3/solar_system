#version 330 core

in vec3 frag_position;
in vec2 frag_uv;
in vec3 frag_normal;

out vec4 color;

uniform sampler2D tex;

void main()
{
    color = texture(tex, frag_uv);
}