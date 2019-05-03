#version 330 core

in vec2 frag_uv;
out vec4 color;

uniform vec4 source;
uniform vec4 destination;
uniform vec4 text_color;
uniform sampler2D font;

void main()
{
    vec4 font_sample = texture(font, frag_uv).rrrr;
    color.r = font_sample.r;
    color.g = font_sample.g;
    color.b = font_sample.b;
    color.a = font_sample.a;
    color *= text_color;
}