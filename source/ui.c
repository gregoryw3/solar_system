internal b32
UIIDEqual(UIID id1, UIID id2)
{
    return (id1.primary == id2.primary &&
            id1.secondary == id2.secondary);
}

internal UIID
UIIDNull(void)
{
    UIID id = { -1, -1 };
    return id;
}

internal UIID
UIIDNonInteractable(void)
{
    UIID id = { -2, -2 };
    return id;
}

internal UIID
UIGenerateID(UI *ui, char *text)
{
    UIID result = {0};
    
    u32 hash = 5381;
    int c;
    
    while(c = *text++)
    {
        hash = ((hash << 5) + hash) + c;
    }
    
    result.primary = (i32)(hash % UI_WIDGET_MAX);
    result.secondary = ui->secondary_id_counter_table[result.primary]++;
    
    return result;
}

internal void
UIInit(UI *ui)
{
    ui->hot = UIIDNull();
    ui->active = UIIDNull();
}

internal void
UIBeginFrame(UI *ui, UIInputData *input, MemoryArena *widget_arena)
{
    ui->mouse_x = input->mouse_x;
    ui->mouse_y = input->mouse_y;
    ui->left_mouse_down = input->left_mouse_down;
    
    ui->widget_count = 0;
    ui->widget_arena = widget_arena;
    
    ui->auto_layout_state.rect = v4(0, 0, 0, 0);
    ui->auto_layout_state.column = 1;
    
    ui->x_position_stack_size = 0;
    ui->y_position_stack_size = 0;
    ui->width_stack_size = 0;
    ui->height_stack_size = 0;
    ui->grouping_mode_stack_size = 0;
}

internal void
UIEndFrame(UI *ui)
{
    for(u32 i = 0; i < ui->widget_count; ++i)
    {
        UIWidget *widget = ui->widgets + i;
        UIID id = widget->id;
        
        widget->hot_transition += ((f32)(!!UIIDEqual(ui->hot, id)) - widget->hot_transition) * app->delta_t * 4.f;
        widget->active_transition += ((f32)(!!UIIDEqual(ui->active, id)) - widget->active_transition) * app->delta_t * 4.f;
        
        v4 rect = widget->rect;
        char *text = widget->text;
        ui->secondary_id_counter_table[id.primary] = 0;
        
        v4 text_color = {
            0.6f + widget->hot_transition*0.4f,
            0.6f + widget->hot_transition*0.4f,
            0.6f + widget->hot_transition*0.4f,
            0.6f + widget->hot_transition*0.4f,
        };
        
        rect.y += 4.f * widget->active_transition;
        
        switch(widget->type)
        {
            case UI_WIDGET_TYPE_button:
            {
                RenderText(&app->font, text_color, v2(rect.x, rect.y), text);
                break;
            }
            case UI_WIDGET_TYPE_checkbox:
            {
                RenderText(&app->font, text_color, v2(rect.x, rect.y), AllocateCStringOnMemoryArena(ui->widget_arena,
                                                                                                    "%s %s", text, widget->checkbox.checked ? "(X)" : "( )"));
                break;
            }
            case UI_WIDGET_TYPE_slider:
            {
                RenderText(&app->font, text_color, v2(rect.x, rect.y), AllocateCStringOnMemoryArena(ui->widget_arena,
                                                                                                    "%s: %.5f", text, widget->slider.slider_value));
                break;
            }
            case UI_WIDGET_TYPE_label:
            {
                RenderText(&app->font, v4(0.4f, 0.4f, 0.4f, 0.4f), v2(rect.x, rect.y), text);
                break;
            }
            default: break;
        }
        
    }
}

internal void
UIPushX(UI *ui, f32 x)
{
    HardAssert(ui->x_position_stack_size < UI_STACK_MAX);
    ui->x_position_stack[ui->x_position_stack_size++].x = ui->auto_layout_state.rect.x;
    if(ui->x_position_stack_size == 1)
    {
        ui->auto_layout_state.rect.x = x;
    }
    else
    {
        ui->auto_layout_state.rect.x += x;
    }
}

internal void
UIPopX(UI *ui)
{
    if(ui->x_position_stack_size > 0)
    {
        ui->auto_layout_state.rect.x = ui->x_position_stack[--ui->x_position_stack_size].x;
    }
}

internal void
UIPushY(UI *ui, f32 y)
{
    HardAssert(ui->y_position_stack_size < UI_STACK_MAX);
    ui->y_position_stack[ui->y_position_stack_size++].y = ui->auto_layout_state.rect.y;
    if(ui->y_position_stack_size == 1)
    {
        ui->auto_layout_state.rect.y = y;
    }
    else
    {
        ui->auto_layout_state.rect.y += y;
    }
}

internal void
UIPopY(UI *ui)
{
    if(ui->y_position_stack_size > 0)
    {
        ui->auto_layout_state.rect.y = ui->y_position_stack[--ui->y_position_stack_size].y;
    }
}

internal void
UIPushWidth(UI *ui, f32 width)
{
    HardAssert(ui->width_stack_size < UI_STACK_MAX);
    ui->width_stack[ui->width_stack_size++].width = ui->auto_layout_state.rect.width;
    ui->auto_layout_state.rect.width = width;
}

internal void
UIPopWidth(UI *ui)
{
    if(ui->width_stack_size > 0)
    {
        ui->auto_layout_state.rect.width = ui->width_stack[--ui->width_stack_size].width;
    }
}

internal void
UIPushHeight(UI *ui, f32 height)
{
    HardAssert(ui->height_stack_size < UI_STACK_MAX);
    ui->height_stack[ui->height_stack_size++].height = ui->auto_layout_state.rect.height;
    ui->auto_layout_state.rect.height = height;
}

internal void
UIPopHeight(UI *ui)
{
    if(ui->height_stack_size > 0)
    {
        ui->auto_layout_state.rect.height = ui->height_stack[--ui->height_stack_size].height;
    }
}

internal void
UIPushPosition(UI *ui, v2 pos)
{
    UIPushX(ui, pos.x);
    UIPushY(ui, pos.y);
}

internal void
UIPopPosition(UI *ui)
{
    UIPopX(ui);
    UIPopY(ui);
}

internal void
UIPushSize(UI *ui, v2 size)
{
    UIPushWidth(ui, size.x);
    UIPushHeight(ui, size.y);
}

internal void
UIPopSize(UI *ui)
{
    UIPopWidth(ui);
    UIPopHeight(ui);
}

internal void
UIPushGroupingMode(UI *ui, b32 column)
{
    HardAssert(ui->grouping_mode_stack_size < UI_STACK_MAX);
    ui->grouping_mode_stack[ui->grouping_mode_stack_size++].column = ui->auto_layout_state.column;
    ui->auto_layout_state.column = column;
}

internal void
UIPopGroupingMode(UI *ui)
{
    if(ui->grouping_mode_stack_size > 0)
    {
        ui->auto_layout_state.column = ui->grouping_mode_stack[--ui->grouping_mode_stack_size].column;
    }
}

internal void
UIPushColumn(UI *ui, v2 position, v2 widget_size)
{
    UIPushGroupingMode(ui, 1);
    UIPushPosition(ui, position);
    UIPushSize(ui, widget_size);
}

internal void
UIPopColumn(UI *ui)
{
    UIPopGroupingMode(ui);
    UIPopPosition(ui);
    UIPopSize(ui);
}

internal void
UIPushRow(UI *ui, v2 position, v2 widget_size)
{
    UIPushGroupingMode(ui, 0);
    UIPushPosition(ui, position);
    UIPushSize(ui, widget_size);
}

internal void
UIPopRow(UI *ui)
{
    UIPopGroupingMode(ui);
    UIPopPosition(ui);
    UIPopSize(ui);
}

internal v4
_UIGetAutoLayoutRectAndAdvance(UI *ui)
{
    v4 rect = ui->auto_layout_state.rect;
    
    if(ui->auto_layout_state.column)
    {
        ui->auto_layout_state.rect.y += ui->auto_layout_state.rect.height;
    }
    else
    {
        ui->auto_layout_state.rect.x += ui->auto_layout_state.rect.width;
    }
    
    return rect;
}

internal b32
_UIButton(UI *ui, UIID id, v4 rect, char *text)
{
    b32 clicked = 0;
    
    b32 mouse_over = V4RectHasPoint(rect, v2(ui->mouse_x, ui->mouse_y));
    
    if(UIIDEqual(ui->hot, id))
    {
        if(mouse_over)
        {
            if(ui->left_mouse_down)
            {
                ui->active = id;
            }
        }
        else
        {
            ui->hot = UIIDNull();
        }
    }
    else
    {
        if(mouse_over && UIIDEqual(ui->hot, UIIDNull()))
        {
            ui->hot = id;
        }
    }
    
    if(UIIDEqual(ui->active, id))
    {
        if(!ui->left_mouse_down)
        {
            clicked = mouse_over;
            ui->active = UIIDNull();
        }
    }
    
    UIWidget *widget = ui->widgets + ui->widget_count++;
    widget->id = id;
    widget->type = UI_WIDGET_TYPE_button;
    widget->rect = rect;
    widget->text = AllocateCStringOnMemoryArena(ui->widget_arena, "%s", text);
    return clicked;
}

internal b32
UIButton(UI *ui, char *text)
{
    v4 rect = _UIGetAutoLayoutRectAndAdvance(ui);
    UIID id = UIGenerateID(ui, text);
    return _UIButton(ui, id, rect, text);
}

internal b32
_UICheckbox(UI *ui, UIID id, v4 rect, char *text, b32 value)
{
    b32 clicked = 0;
    
    b32 mouse_over = V4RectHasPoint(rect, v2(ui->mouse_x, ui->mouse_y));
    
    if(UIIDEqual(ui->hot, id))
    {
        if(mouse_over)
        {
            if(ui->left_mouse_down)
            {
                ui->active = id;
            }
        }
        else
        {
            ui->hot = UIIDNull();
        }
    }
    else
    {
        if(mouse_over && UIIDEqual(ui->hot, UIIDNull()))
        {
            ui->hot = id;
        }
    }
    
    if(UIIDEqual(ui->active, id))
    {
        if(!ui->left_mouse_down)
        {
            clicked = mouse_over;
            ui->active = UIIDNull();
        }
    }
    
    if(clicked)
    {
        value = !value;
    }
    
    UIWidget *widget = ui->widgets + ui->widget_count++;
    widget->id = id;
    widget->type = UI_WIDGET_TYPE_checkbox;
    widget->rect = rect;
    widget->text = AllocateCStringOnMemoryArena(ui->widget_arena, "%s", text);
    widget->checkbox.checked = value;
    
    return value;
}

internal b32
UICheckbox(UI *ui, char *text, b32 value)
{
    v4 rect = _UIGetAutoLayoutRectAndAdvance(ui);
    UIID id = UIGenerateID(ui, text);
    return _UICheckbox(ui, id, rect, text, value);
}

internal f32
_UISlider(UI *ui, UIID id, v4 rect, char *text, f32 value, f32 low, f32 high)
{
    b32 mouse_over = V4RectHasPoint(rect, v2(ui->mouse_x, ui->mouse_y));
    
    f32 slider_percentage = (value - low) / (high - low);
    
    if(UIIDEqual(ui->hot, id))
    {
        if(mouse_over)
        {
            if(ui->left_mouse_down)
            {
                ui->active = id;
            }
        }
        else
        {
            ui->hot = UIIDNull();
        }
    }
    else
    {
        if(mouse_over && UIIDEqual(ui->hot, UIIDNull()))
        {
            ui->hot = id;
        }
    }
    
    if(UIIDEqual(ui->active, id))
    {
        if(ui->left_mouse_down)
        {
            slider_percentage = (ui->mouse_x - rect.x) / rect.width;
        }
        else
        {
            ui->active = UIIDNull();
        }
    }
    
    if(slider_percentage < 0) { slider_percentage = 0.f; }
    if(slider_percentage > 1) { slider_percentage = 1.f; }
    
    value = slider_percentage * (high - low) + low;
    
    UIWidget *widget = ui->widgets + ui->widget_count++;
    widget->id = id;
    widget->type = UI_WIDGET_TYPE_slider;
    widget->rect = rect;
    widget->text = AllocateCStringOnMemoryArena(ui->widget_arena, "%s", text);
    widget->slider.slider_percentage = slider_percentage;
    widget->slider.slider_value = value;
    
    return value;
}

internal f32
UISlider(UI *ui, char *text, f32 value, f32 low, f32 high)
{
    v4 rect = _UIGetAutoLayoutRectAndAdvance(ui);
    UIID id = UIGenerateID(ui, text);
    return _UISlider(ui, id, rect, text, value, low, high);
}

internal void
_UILabel(UI *ui, v4 rect, char *text)
{
    UIWidget *widget = ui->widgets + ui->widget_count++;
    widget->id = UIIDNonInteractable();
    widget->type = UI_WIDGET_TYPE_button;
    widget->rect = rect;
    widget->text = AllocateCStringOnMemoryArena(ui->widget_arena, "%s", text);
}

internal void
UILabel(UI *ui, char *text)
{
    v4 rect = _UIGetAutoLayoutRectAndAdvance(ui);
    _UILabel(ui, rect, text);
}
