#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_ONLY_JPEG
#define STBI_ONLY_BMP
#define STBI_NO_SIMD
#include "ext/stb_image.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "ext/stb_truetype.h"

#include "utilities.h"
#include "opengl.h"
#include "ui.h"
#include "app.h"
#include "debug.h"
#include "generated_model.h"

#include "opengl.c"
#include "ui.c"

internal v3
GetCelestialBodyPosition(CelestialBody *body, f32 time_in_years)
{
    v3 position = {0};
    
    if(body->orbitee)
    {
        v3 orbitee_position = GetCelestialBodyPosition(body->orbitee, time_in_years);
        position.x = orbitee_position.x + body->orbital_distance*cosf(2.f * PIf * time_in_years / body->orbital_period);
        position.z = orbitee_position.z + body->orbital_distance*sinf(2.f * PIf * time_in_years / body->orbital_period);
    }
    
    return position;
}

internal void
Update(Platform *platform_)
{
    platform = platform_;
    
    if(platform->initialized == 0)
    {
        platform->initialized = 1;
        
        app = platform->permanent_storage;
        app->permanent_arena = MemoryArenaInit(platform->permanent_storage, platform->permanent_storage_size);
        app->scratch_arena = MemoryArenaInit(platform->scratch_storage, platform->scratch_storage_size);
        MemoryArenaAllocate(&app->permanent_arena, sizeof(App));
        LoadAllOpenGLProcedures(platform);
        
        UIInit(&app->ui);
        
        // NOTE(rjf): Init rendering data
        {
            glGenVertexArrays(1, &app->text_char_vao);
        }
        
        // NOTE(rjf): Load planet model information to GPU
        {
            app->sphere_vertex_count = sizeof(global_planet_model_data) / (sizeof(f32) * 8);
            glGenVertexArrays(1, &app->sphere_vao);
            glBindVertexArray(app->sphere_vao);
            glGenBuffers(1, &app->sphere_vertex_buffer);
            glBindBuffer(GL_ARRAY_BUFFER, app->sphere_vertex_buffer);
            glBufferData(GL_ARRAY_BUFFER, sizeof(global_planet_model_data), global_planet_model_data, GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(f32)*8, 0);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(f32)*8, (void *)(sizeof(f32)*3));
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(f32)*8, (void *)(sizeof(f32)*5));
            glBindVertexArray(0);
        }
        
        // NOTE(rjf): Initialize all shaders
        {
            // NOTE(rjf): Planet/sky/star shaders
            {
                OpenGLShaderInput inputs[] = {
                    { 0, "vert_position" },
                    { 1, "vert_uv" },
                    { 2, "vert_normal" },
                };
                
                OpenGLShaderOutput outputs[] = {
                    { 0, "color" },
                };
                
                app->planet_shader = LoadOpenGLShader(inputs, ArrayCount(inputs),
                                                      outputs, ArrayCount(outputs),
                                                      "data/planet.vert", "data/planet.frag");
                
                app->star_shader = LoadOpenGLShader(inputs, ArrayCount(inputs),
                                                    outputs, ArrayCount(outputs),
                                                    "data/star.vert", "data/star.frag");
                
                app->sky_shader = LoadOpenGLShader(inputs, ArrayCount(inputs),
                                                   outputs, ArrayCount(outputs),
                                                   "data/sky.vert", "data/sky.frag");
            }
            
            // NOTE(rjf): Text shader
            {
                OpenGLShaderOutput outputs[] = {
                    { 0, "color" },
                };
                
                app->text_shader = LoadOpenGLShader(0, 0,
                                                    outputs, ArrayCount(outputs),
                                                    "data/text.vert", "data/text.frag");
            }
        }
        
        // NOTE(rjf): Load resources
        {
            app->sky_texture = LoadTexture("data/sky.jpg");
            app->font = LoadFont("data/Inconsolata.ttf");
        }
        
        app->time_in_years = 24.334f;
        app->time_rate = 1.f / 365.f;
        
        // NOTE(rjf): Initialize all celestial bodies
        {
            struct
            {
                char *name;
                char *description;
                f32 average_temperature;
                char *day_texture_filename;
                char *night_texture_filename;
                int orbitee_index;
                f32 orbital_distance;
                f32 orbital_period;
                f32 rotational_period;
                f32 scale;
                f32 radius;
                GLuint *shader;
            }
            bodies[] = {
                
#define Name(a) (a)
#define Description(a) (a)
#define Temperature(a) (a)
#define DayTexture(a) (a)
#define NightTexture(a) (a)
#define OrbiteeIndex(a) (a)
#define OrbitalDistance(a) (a)
#define OrbitalPeriod(a) (a)
#define RotationalPeriod(a) (a)
#define Scale(a) (a)
#define Radius(a) (a)
                
                {
                    Name             ("Sun"),
                    Description      (""),
                    Temperature      (5778.f),
                    DayTexture       ("data/sun.jpg"),
                    NightTexture     (0),
                    OrbiteeIndex     (-1),
                    OrbitalDistance  (0.f),
                    OrbitalPeriod    (1.f),
                    RotationalPeriod (0.067041096),
                    Scale            (109.197724319f),
                    Radius           (109.197724319f),
                    &app->star_shader,
                },
                
                {
                    Name             ("Mercury"),
                    Description      (""),
                    Temperature      (699.f),
                    DayTexture       ("data/mercury.jpg"),
                    NightTexture     (0),
                    OrbiteeIndex     (0),
                    OrbitalDistance  (300.f),
                    OrbitalPeriod    (88.f / 365.f),
                    RotationalPeriod (1.f / 365.f),
                    Scale            (20.383f),
                    Radius           (0.383f),
                    &app->planet_shader,
                },
                
                {
                    Name             ("Venus"),
                    Description      (""),
                    Temperature      (735.15f),
                    DayTexture       ("data/venus.jpg"),
                    NightTexture     (0),
                    OrbiteeIndex     (0),
                    OrbitalDistance  (600.f),
                    OrbitalPeriod    (0.615f),
                    RotationalPeriod (-244.f / 365.f),
                    Scale            (30.949f),
                    Radius           (0.949f),
                    &app->planet_shader,
                },
                
                {
                    Name             ("Earth"),
                    Description      (""),
                    Temperature      (288.f),
                    DayTexture       ("data/earth_day.jpg"),
                    NightTexture     ("data/earth_night.jpg"),
                    OrbiteeIndex     (0),
                    OrbitalDistance  (1000.f),
                    OrbitalPeriod    (1.f),
                    RotationalPeriod (1.f / 365.f),
                    Scale            (40.f),
                    Radius           (1.f),
                    &app->planet_shader,
                },
                
                {
                    Name             ("Moon"),
                    Description      (""),
                    Temperature      (10.f),
                    DayTexture       ("data/moon.jpg"),
                    NightTexture     (0),
                    OrbiteeIndex     (3),
                    OrbitalDistance  (100.f),
                    OrbitalPeriod    (1.f / 12.f),
                    RotationalPeriod (-1.f / 12.f),
                    Scale            (10.2724f),
                    Radius           (0.2724f),
                    &app->planet_shader,
                },
                
                {
                    Name             ("Mars"),
                    Description      (""),
                    Temperature      (293.15f),
                    DayTexture       ("data/mars.jpg"),
                    NightTexture     (0),
                    OrbiteeIndex     (0),
                    OrbitalDistance  (1500.f),
                    OrbitalPeriod    (687.f / 365.f),
                    RotationalPeriod (1.f / 365.f),
                    Scale            (20.53f),
                    Radius           (0.532f),
                    &app->planet_shader,
                },
                
#undef Name 
#undef Description 
#undef Temperature 
#undef DayTexture 
#undef NightTexture 
#undef OrbiteeIndex 
#undef OrbitalDistance 
#undef OrbitalPeriod 
#undef RotationalPeriod 
#undef Scale 
#undef Radius
                
            };
            
            app->celestial_body_count = ArrayCount(bodies);
            app->celestial_bodies = MemoryArenaAllocate(&app->permanent_arena,
                                                        ArrayCount(bodies) * sizeof(CelestialBody));
            
            for(int i = 0; i < ArrayCount(bodies); ++i)
            {
                CelestialBody body = {0};
                {
                    body.name = bodies[i].name;
                    body.description = bodies[i].description;
                    body.average_temperature = bodies[i].average_temperature;
                    if(bodies[i].orbitee_index >= 0)
                    {
                        body.orbitee = app->celestial_bodies + bodies[i].orbitee_index;
                    }
                    body.orbital_distance = bodies[i].orbital_distance;
                    body.orbital_period = bodies[i].orbital_period;
                    body.rotational_period = bodies[i].rotational_period;
                    body.scale = bodies[i].scale;
                    body.radius = bodies[i].radius;
                }
                
                if(bodies[i].day_texture_filename)
                {
                    body.day_texture = LoadTexture(bodies[i].day_texture_filename);
                }
                
                if(bodies[i].night_texture_filename)
                {
                    body.night_texture = LoadTexture(bodies[i].night_texture_filename);
                }
                
                body.shader = bodies[i].shader;
                
                app->celestial_bodies[i] = body;
            }
        }
        
        app->camera_target_celestial_body = app->celestial_bodies + 0;
        app->camera_distance = app->camera_distance_target = 500.f;
    }
    
    // NOTE(rjf): Load data from platform
    {
        app->render_w = platform->window_width;
        app->render_h = platform->window_height;
        app->delta_t = platform->current_time - platform->last_time;
    }
    
    // NOTE(rjf): Update
    UIInputData ui_input = {0};
    {
        ui_input.mouse_x = platform->mouse_x;
        ui_input.mouse_y = platform->mouse_y;
        ui_input.left_mouse_down = platform->left_mouse_down;
    }
    MemoryArenaClear(&app->scratch_arena);
    UIBeginFrame(&app->ui, &ui_input, &app->scratch_arena);
    {
        app->time_in_years += app->delta_t * app->time_rate;
        
        UIPushColumn(&app->ui, v2(16, 16), v2(400, 30));
        {
            UILabel(&app->ui, "Go To Celestial Body");
            UILabel(&app->ui, "--------------------");
            for(u32 i = 0; i < app->celestial_body_count; ++i)
            {
                if(UIButton(&app->ui, app->celestial_bodies[i].name))
                {
                    v3 old_target = GetCelestialBodyPosition(app->camera_target_celestial_body, app->time_in_years);
                    app->camera_target_celestial_body = app->celestial_bodies + i;
                    v3 new_target = GetCelestialBodyPosition(app->camera_target_celestial_body, app->time_in_years);
                    app->camera_target_offset.x = old_target.x - new_target.x;
                    app->camera_target_offset.y = old_target.y - new_target.y;
                    app->camera_target_offset.z = old_target.z - new_target.z;
                }
            }
            UILabel(&app->ui, "");
            UILabel(&app->ui, "");
            UILabel(&app->ui, "Settings");
            UILabel(&app->ui, "--------------------");
            app->time_rate = UISlider(&app->ui, "Time Speed (Years Per Second)", app->time_rate, 1.f / 365.f, 1.f);
            
            UILabel(&app->ui, "");
            UILabel(&app->ui, "");
            UILabel(&app->ui, AllocateCStringOnMemoryArena(&app->scratch_arena, "Information (%s)", app->camera_target_celestial_body->name));
            UILabel(&app->ui, "--------------------");
            UILabel(&app->ui, AllocateCStringOnMemoryArena(&app->scratch_arena, "Average Temperature: %.2f K", app->camera_target_celestial_body->average_temperature));
            UILabel(&app->ui, AllocateCStringOnMemoryArena(&app->scratch_arena, "Radius: %.2f Earth radii", app->camera_target_celestial_body->radius));
            UILabel(&app->ui, AllocateCStringOnMemoryArena(&app->scratch_arena, "Orbital Period: %.2f years", app->camera_target_celestial_body->orbital_period));
            UILabel(&app->ui, AllocateCStringOnMemoryArena(&app->scratch_arena, "Rotational Period: %.2f years", fabsf(app->camera_target_celestial_body->rotational_period)));
        }
        UIPopColumn(&app->ui);
        
        // NOTE(rjf): Update camera
        {
            if(UIIDEqual(app->ui.hot, UIIDNull()))
            {
                if(app->dragging_camera)
                {
                    if(platform->left_mouse_down)
                    {
                        f32 sensitivity = 1 / 500.f;
                        app->target_to_camera_yaw_target += sensitivity * (platform->mouse_x - platform->last_mouse_x);
                        app->target_to_camera_pitch_target += sensitivity * (platform->mouse_y - platform->last_mouse_y);
                    }
                    else
                    {
                        app->dragging_camera = 0;
                    }
                }
                else
                {
                    if(platform->left_mouse_down)
                    {
                        app->dragging_camera = 1;
                    }
                }
            }
            
            if(app->target_to_camera_pitch_target >= PIf/2)
            {
                app->target_to_camera_pitch_target = PIf/2;
            }
            else if(app->target_to_camera_pitch_target <= -PIf/2)
            {
                app->target_to_camera_pitch_target = -PIf/2;
            }
            
            app->camera_distance_target -= platform->mouse_scroll_y * 0.5f;
            
            if(app->camera_distance_target < 10.f)
            {
                app->camera_distance_target = 10.f;
            }
            
            app->target_to_camera_yaw += (app->target_to_camera_yaw_target - app->target_to_camera_yaw) * app->delta_t * 16.f;
            app->target_to_camera_pitch += (app->target_to_camera_pitch_target - app->target_to_camera_pitch) * app->delta_t * 16.f;
            app->camera_distance += (app->camera_distance_target - app->camera_distance) * app->delta_t * 16.f;
            app->camera_target_offset.x += (0 - app->camera_target_offset.x)*2.f*(app->delta_t);
            app->camera_target_offset.y += (0 - app->camera_target_offset.y)*2.f*(app->delta_t);
            app->camera_target_offset.z += (0 - app->camera_target_offset.z)*2.f*(app->delta_t);
        }
        
        m4 view;
        m4 projection;
        
        // NOTE(rjf): Initialize view/projection matrices
        {
            f32 yaw = app->target_to_camera_yaw;
            f32 pitch = app->target_to_camera_pitch;
            f32 distance = app->camera_distance;
            
            CelestialBody *target_body = app->camera_target_celestial_body;
            v3 target = GetCelestialBodyPosition(target_body, app->time_in_years);
            target.x += app->camera_target_offset.x;
            target.y += app->camera_target_offset.y;
            target.z += app->camera_target_offset.z;
            
            view = M4LookAt(
                v3(target.x + distance * cosf(yaw) * cosf(pitch),
                   target.y + distance * sinf(pitch),
                   target.z + distance * sinf(yaw) * cosf(pitch)),
                target,
                v3(0, 1, 0));
            projection = M4Perspective(90.f, app->render_w / app->render_h,
                                       10.f, 10000.f);
        }
        
        glDepthMask(GL_TRUE);
        glEnable(GL_DEPTH_TEST);
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, (GLsizei)app->render_w, (GLsizei)app->render_h);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        
        // NOTE(rjf): Render sky
        {
            glDisable(GL_DEPTH_TEST);
            glDepthMask(GL_FALSE);
            
            GLuint shader = app->sky_shader;
            
            m4 sky_view = view;
            sky_view.elements[3][0] = 0;
            sky_view.elements[3][1] = 0;
            sky_view.elements[3][2] = 0;
            
            m4 model;
            model = M4InitD(1.f);
            model = M4MultiplyM4(M4ScaleV3(v3(20.f, 20.f, 20.f)), model);
            model = M4MultiplyM4(M4TranslateV3(v3(0.f, 0.f, 0.f)), model);
            
            glUseProgram(shader);
            glBindVertexArray(app->sphere_vao);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, app->sky_texture.id);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glUniform1i(glGetUniformLocation(shader, "tex"), 0);
            glUniformMatrix4fv(glGetUniformLocation(shader, "model"),
                               1, 0, &model.elements[0][0]);
            glUniformMatrix4fv(glGetUniformLocation(shader, "view"),
                               1, 0, &sky_view.elements[0][0]);
            glUniformMatrix4fv(glGetUniformLocation(shader, "projection"),
                               1, 0, &projection.elements[0][0]);
            glDrawArrays(GL_TRIANGLES, 0, app->sphere_vertex_count);
            glBindVertexArray(0);
            glUseProgram(0);
            
            glDepthMask(GL_TRUE);
            glEnable(GL_DEPTH_TEST);
        }
        
        // NOTE(rjf): Render celestial bodies
        for(u32 i = 0; i < app->celestial_body_count; ++i)
        {
            CelestialBody *body = app->celestial_bodies + i;
            GLuint shader = *body->shader;
            
            v3 position = GetCelestialBodyPosition(body, app->time_in_years);
            
            m4 model;
            model = M4InitD(1.f);
            model = M4MultiplyM4(M4Rotate(2 * PIf * app->time_in_years / body->rotational_period,
                                          v3(0, 1, 0)), model);
            model = M4MultiplyM4(M4ScaleV3(v3(body->scale, body->scale, body->scale)), model);
            model = M4MultiplyM4(M4TranslateV3(position), model);
            
            glUseProgram(shader);
            glBindVertexArray(app->sphere_vao);
            
            glUniform1i(glGetUniformLocation(shader, "day_texture"), 0);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, body->day_texture.id);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            
            glUniform1i(glGetUniformLocation(shader, "night_texture"), 1);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, body->night_texture.id);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            
            glUniform3f(glGetUniformLocation(shader, "star_to_planet_vector"),
                        position.x, position.y, position.z);
            glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, 0, &model.elements[0][0]);
            glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, 0, &view.elements[0][0]);
            glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, 0, &projection.elements[0][0]);
            glDrawArrays(GL_TRIANGLES, 0, app->sphere_vertex_count);
            glBindVertexArray(0);
            glUseProgram(0);
        }
        
        glDisable(GL_DEPTH_TEST);
        UIEndFrame(&app->ui);
        
        platform->SwapBuffers();
    }
    
    platform->mouse_scroll_x = 0.f;
    platform->mouse_scroll_y = 0.f;
    platform->last_mouse_x = platform->mouse_x;
    platform->last_mouse_y = platform->mouse_y;
    platform->last_time = platform->current_time;
}

internal void
RenderText(Font *font, v4 color, v2 position, char *text)
{
    position.y += 24;
    
    GLuint shader = app->text_shader;
    glUseProgram(shader);
    glBindVertexArray(app->text_char_vao);
    
    glUniform1i(glGetUniformLocation(shader, "font"), 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, font->texture.id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    for(int i = 0; text[i]; ++i)
    {
        stbtt_aligned_quad quad;
        stbtt_GetBakedQuad(font->char_data, 512, 512, text[i]-32,  &position.x, &position.y, &quad, 1);
        
        v4 source = {
            quad.s0, quad.t0,
            quad.s1 - quad.s0,
            quad.t1 - quad.t0,
        };
        
        v4 destination = {
            quad.x0, quad.y0,
            quad.x1 - quad.x0,
            quad.y1 - quad.y0,
        };
        
        glUniform2f(glGetUniformLocation(shader, "resolution"), app->render_w, app->render_h);
        glUniform4f(glGetUniformLocation(shader, "text_color"), color.x, color.y, color.z, color.w);
        glUniform4f(glGetUniformLocation(shader, "source"), source.x, source.y, source.z, source.w);
        glUniform4f(glGetUniformLocation(shader, "destination"), destination.x, destination.y, destination.z, destination.w);
        
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
    
    glBindVertexArray(0);
    glUseProgram(0);
}