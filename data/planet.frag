#version 330 core

in vec3 frag_position;
in vec2 frag_uv;
in vec3 frag_normal;

out vec4 color;

uniform sampler2D day_texture;
uniform sampler2D night_texture;
uniform vec3 star_to_planet_vector;

void main()
{
    float light_factor = -dot(normalize(frag_normal), normalize(star_to_planet_vector));
    
    vec3 day_sample = texture(day_texture, frag_uv).rgb * light_factor;
    vec3 night_sample = texture(night_texture, frag_uv).rgb * (1-light_factor);
    color.rgb = day_sample + night_sample;
    color.a = 1;
}