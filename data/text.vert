#version 330 core

out vec2 frag_uv;

uniform vec4 source;
uniform vec4 destination;
uniform vec2 resolution;
uniform sampler2D font;

void main()
{
    vec2 vertices[] = vec2[](
        vec2(0, 0),
        vec2(0, 1),
        vec2(1, 0),
        vec2(1, 1));
    
    vec4 position = vec4(vertices[gl_VertexID], 0, 1);
    position.xy *= destination.zw;
    position.xy += destination.xy;
    position.xy /= resolution;
    position.xy *= 2;
    position.xy -= vec2(1, 1);
    position.y *= -1;
    
    vec2 uv = vertices[gl_VertexID];
    uv *= source.zw;
    uv += source.xy;
    
    frag_uv = uv;
    
    gl_Position = position;
}