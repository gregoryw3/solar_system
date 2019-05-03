typedef struct CelestialBody CelestialBody;
typedef struct CelestialBody
{
    char *name;
    char *description;
    f32 average_temperature;
    CelestialBody *orbitee;
    f32 orbital_distance;
    f32 orbital_period;
    f32 rotational_period;
    f32 scale;
    f32 radius;
    Texture day_texture;
    Texture night_texture;
    GLuint *shader;
}
CelestialBody;

typedef struct App App;
struct App
{
    MemoryArena permanent_arena;
    MemoryArena scratch_arena;
    
    f32 render_w;
    f32 render_h;
    f32 delta_t;
    
    u32 sphere_vertex_count;
    GLuint sphere_vao;
    GLuint sphere_vertex_buffer;
    
    GLuint text_char_vao;
    
    GLuint planet_shader;
    GLuint star_shader;
    GLuint sky_shader;
    GLuint text_shader;
    
    Font font;
    Texture sky_texture;
    
    u32 celestial_body_count;
    CelestialBody *celestial_bodies;
    f32 time_in_years;
    f32 time_rate;
    
    b32 dragging_camera;
    f32 target_to_camera_yaw;
    f32 target_to_camera_pitch;
    f32 target_to_camera_yaw_target;
    f32 target_to_camera_pitch_target;
    f32 camera_distance;
    f32 camera_distance_target;
    CelestialBody *camera_target_celestial_body;
    v3 camera_target_offset;
    
    UI ui;
};

global App *app = 0;
global Platform *platform = 0;

internal void RenderText(Font *font, v4 color, v2 position, char *text);