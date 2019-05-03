typedef struct MemoryArena
{
    void *memory;
    u32 memory_size;
    u32 alloc_position;
}
MemoryArena;

static MemoryArena
MemoryArenaInit(void *memory, u32 memory_size)
{
    MemoryArena arena = {
        memory,
        memory_size,
        0,
    };
    return arena;
}

static void
MemoryArenaClear(MemoryArena *arena)
{
    arena->alloc_position = 0;
}

static void *
MemoryArenaAllocate(MemoryArena *arena, u32 size)
{
    void *memory = 0;
    if(arena->alloc_position + size <= arena->memory_size)
    {
        memory = (char *)arena->memory + arena->alloc_position;
        arena->alloc_position += size;
    }
    return memory;
}

static char *
AllocateCStringOnMemoryArena(MemoryArena *arena, char *format, ...)
{
    char *result = 0;
    
    va_list args;
    
    va_start(args, format);
    u32 needed_bytes = vsnprintf(0, 0, format, args)+1;
    va_end(args);
    
    result = MemoryArenaAllocate(arena, needed_bytes);
    
    if(result)
    {
        va_start(args, format);
        vsnprintf(result, needed_bytes, format, args);
        va_end(args);
    }
    
    return result;
}