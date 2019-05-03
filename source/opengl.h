#define GLProc(name, type) PFNGL##type##PROC gl##name = 0;
#include "opengl_procedure_list.h"

internal void
LoadAllOpenGLProcedures(Platform *platform)
{
#define GLProc(name, type) gl##name = platform->LoadOpenGLProcedure("gl" #name);
#include "opengl_procedure_list.h"
}

typedef struct Texture
{
    int width;
    int height;
    GLuint id;
}
Texture;

typedef struct Font
{
    Texture texture;
    stbtt_bakedchar char_data[96];
}
Font;

typedef struct OpenGLShaderInput
{
    int index;
    char *name;
}
OpenGLShaderInput;

typedef struct OpenGLShaderOutput
{
    int index;
    char *name;
}
OpenGLShaderOutput;
