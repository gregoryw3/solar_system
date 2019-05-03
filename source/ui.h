#define UI_WIDGET_MAX 1024
#define UI_STACK_MAX 32

typedef struct UIID
{
    i32 primary;
    i32 secondary;
}
UIID;

typedef enum UIWidgetType
{
    UI_WIDGET_TYPE_button,
    UI_WIDGET_TYPE_checkbox,
    UI_WIDGET_TYPE_slider,
    UI_WIDGET_TYPE_label,
}
UIWidgetType;

typedef struct UIWidget
{
    UIID id;
    UIWidgetType type;
    v4 rect;
    char *text;
    f32 hot_transition;
    f32 active_transition;
    
    // NOTE(rjf): Widget-type-specific data
    union
    {
        struct Checkbox
        {
            b32 checked;
        }
        checkbox;
        
        struct Slider
        {
            f32 slider_percentage;
            f32 slider_value;
        }
        slider;
    };
    
}
UIWidget;

typedef struct UIInputData
{
    f32 mouse_x;
    f32 mouse_y;
    b32 left_mouse_down;
}
UIInputData;

typedef struct UI
{
    f32 mouse_x;
    f32 mouse_y;
    b32 left_mouse_down;
    
    UIID hot;
    UIID active;
    u32 widget_count;
    UIWidget widgets[UI_WIDGET_MAX];
    i32 secondary_id_counter_table[UI_WIDGET_MAX];
    MemoryArena *widget_arena;
    
    struct
    {
        v4 rect;
        b32 column;
    }
    auto_layout_state;
    
    struct
    {
        f32 x;
    }
    x_position_stack[UI_STACK_MAX];
    u32 x_position_stack_size;
    
    struct
    {
        f32 y;
    }
    y_position_stack[UI_STACK_MAX];
    u32 y_position_stack_size;
    
    struct
    {
        f32 width;
    }
    width_stack[UI_STACK_MAX];
    u32 width_stack_size;
    
    struct
    {
        f32 height;
    }
    height_stack[UI_STACK_MAX];
    u32 height_stack_size;
    
    struct
    {
        b32 column;
    }
    grouping_mode_stack[UI_STACK_MAX];
    u32 grouping_mode_stack_size;
}
UI;
