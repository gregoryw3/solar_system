
static GLuint
InitOpenGLShaderFromData(OpenGLShaderInput *inputs, int input_count,
                         OpenGLShaderOutput *outputs, int output_count,
                         char *vertex, char *fragment)
{
    GLuint program = 0;
    GLuint vertex_shader = 0;
    GLuint fragment_shader = 0;
    
    int info_log_length = 0;
    GLint result = GL_FALSE;
    GLint code_length = 0;
    
    // NOTE(rjf): Compile and attach vertex shader
    {
        vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        code_length = (GLint)CalculateCStringLength(vertex);
        glShaderSource(vertex_shader, 1, &vertex, &code_length);
        glCompileShader(vertex_shader);
        
        // NOTE(rjf): Get vertex shader errors
        {
            glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &result);
            glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &info_log_length);
            if(info_log_length > 1)
            {
                char *vertex_shader_error = MemoryArenaAllocate(&app->scratch_arena, info_log_length);
                glGetShaderInfoLog(vertex_shader, info_log_length, 0, vertex_shader_error);
                Log("%s", vertex_shader_error);
            }
            else
            {
                Log("Vertex shader compiled successfully.");
            }
        }
    }
    
    // NOTE(rjf): Compile and attach fragment shader
    {
        fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
        code_length = (GLint)CalculateCStringLength(fragment);
        glShaderSource(fragment_shader, 1, &fragment, &code_length);
        glCompileShader(fragment_shader);
        
        // NOTE(rjf): Get fragment shader errors
        {
            glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &result);
            glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &info_log_length);
            if(info_log_length > 1)
            {
                char *fragment_shader_error = MemoryArenaAllocate(&app->scratch_arena, info_log_length);
                glGetShaderInfoLog(fragment_shader, info_log_length, 0, fragment_shader_error);
                Log("%s", fragment_shader_error);
            }
            else
            {
                Log("Fragment shader compiled successfully.");
            }
        }
    }
    
    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    
    // NOTE(rjf): Specify shader inputs
    {
        for(int i = 0; i < input_count; ++i)
        {
            glBindAttribLocation(program, inputs[i].index, inputs[i].name);
        }
    }
    
    // NOTE(rjf): Specify shader outputs
    {
        for(int i = 0; i < output_count; ++i)
        {
            glBindFragDataLocation(program, outputs[i].index, outputs[i].name);
        }
    }
    
    // NOTE(rjf): Link shaders
    {
        glLinkProgram(program);
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
    }
    
    glValidateProgram(program);
    
    return program;
}

static Texture
InitTextureFromRawData(void *data, int width, int height)
{
    Texture texture = {0};
    GLuint id = 0;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    texture.id = id;
    texture.width = width;
    texture.height = height;
    return texture;
}

static Texture
InitTextureFromImageData(void *data, int data_size)
{
    int width = 0;
    int height = 0;
    int channels_in_file = 0;
    u8 *raw_data = stbi_load_from_memory((u8 *)data, data_size, &width, &height, &channels_in_file, 4);
    Texture tex = InitTextureFromRawData(raw_data, width, height);
    stbi_image_free(raw_data);
    return tex;
}

static GLuint
LoadOpenGLShader(OpenGLShaderInput *inputs, int input_count,
                 OpenGLShaderOutput *outputs, int output_count,
                 char *vertex_filename, char *fragment_filename)
{
    GLuint id = 0;
    
    char *vert = platform->LoadEntireFileAndNullTerminate(vertex_filename);
    char *frag = platform->LoadEntireFileAndNullTerminate(fragment_filename);
    
    if(vert && frag)
    {
        id = InitOpenGLShaderFromData(inputs, input_count, outputs,
                                      output_count, vert, frag);
        
        platform->FreeFileData(vert);
        platform->FreeFileData(frag);
    }
    
    return id;
}

static Texture
LoadTexture(char *filename)
{
    Texture result = {0};
    int size = 0;
    void *file = platform->LoadEntireFile(filename, &size);
    if(file)
    {
        result = InitTextureFromImageData(file, size);
        platform->FreeFileData(file);
    }
    return result;
}

static Font
InitFontFromTTFData(void *data, int data_size)
{
    Font font = {0};
    unsigned char *temp_bitmap = malloc(512 * 512);
    stbtt_BakeFontBitmap(data, 0, 24.0, temp_bitmap, 512, 512, 32, 96, font.char_data);
    
    GLuint id = 0;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, 512, 512, 0, GL_RED, GL_UNSIGNED_BYTE, temp_bitmap);
    glBindTexture(GL_TEXTURE_2D, 0);
    font.texture.id = id;
    font.texture.width = 512;
    font.texture.height = 512;
    
    free(temp_bitmap);
    return font;
}

static Font
LoadFont(char *filename)
{
    Font result = {0};
    int size = 0;
    void *file = platform->LoadEntireFile(filename, &size);
    if(file)
    {
        result = InitFontFromTTFData(file, size);
        platform->FreeFileData(file);
    }
    return result;
}