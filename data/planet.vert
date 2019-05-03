#version 330 core

in vec3 vert_position;
in vec2 vert_uv;
in vec3 vert_normal;

out vec3 frag_position;
out vec2 frag_uv;
out vec3 frag_normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    vec4 position = vec4(vert_position, 1);
    position = projection * view * model * position;
    gl_Position = position;
    
    vec4 normal = model * vec4(vert_normal, 0);
    
    frag_position = vert_position;
    frag_uv = vert_uv;
    frag_uv.y = 1 - frag_uv.y;
    frag_normal = normal.xyz;
}