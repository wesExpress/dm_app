#define NK_IMPLEMENTATION
#include "gui.h"

void nuklear_clipboard_copy(nk_handle user, const char* text, int len)
{
    NK_UNUSED(user);
    NK_UNUSED(text);
    NK_UNUSED(len);
}

void nuklear_clipboad_paste(nk_handle user, struct nk_text_edit* edit)
{
    NK_UNUSED(user);
    NK_UNUSED(edit);
}

gui_context* gui_init(const char** font_paths, uint8_t* font_sizes, uint8_t font_count, dm_context* context)
{
    gui_context* gui = dm_alloc(sizeof(gui_context));

    // === sampler ===
    dm_sampler_desc sampler_desc = {
        .address_w=DM_SAMPLER_ADDRESS_MODE_BORDER,
        .address_v=DM_SAMPLER_ADDRESS_MODE_BORDER,
        .address_u=DM_SAMPLER_ADDRESS_MODE_BORDER
    };

    if(!dm_create_sampler(sampler_desc, &gui->sampler, context)) { dm_free((void*)gui); return NULL; }

    // === constant buffer ===
    glm_ortho(0,(float)dm_get_window_width(context), (float)dm_get_window_height(context),0, -1,1, gui->ortho);
#ifdef DM_DIRECTX12
    glm_mat4_transpose(gui->ortho);
#endif

    dm_constant_buffer_desc cb_desc = {
        .size=sizeof(mat4),
        .data=&gui->ortho
    };

    if(!dm_create_constant_buffer(cb_desc, &gui->cb, context))  { dm_free((void*)gui); return NULL; }

    gui->screen_width  = dm_get_window_width(context);
    gui->screen_height = dm_get_window_height(context); 

    // === vertex and index buffers ===
    dm_vertex_buffer_desc vb_desc = { 
        .stride=sizeof(nuklear_vertex),
        .size=sizeof(gui->vertices)
    };

    if(!dm_create_vertex_buffer(vb_desc, &gui->vb, context)) { dm_free((void*)gui); return NULL; }

    dm_index_buffer_desc ib_desc = {
        .size=sizeof(gui->indices),
        .index_type=DM_INDEX_BUFFER_INDEX_TYPE_UINT16,
    };

    if(!dm_create_index_buffer(ib_desc, &gui->ib, context)) { dm_free((void*)gui); return NULL; }

    // === pipeline ===
    dm_input_element_desc position_element = {
        .name="POSITION",
        .class=DM_INPUT_ELEMENT_CLASS_PER_VERTEX,
        .format=DM_INPUT_ELEMENT_FORMAT_FLOAT_2,
        .stride=sizeof(nuklear_vertex),
        .offset=offsetof(nuklear_vertex, position),
    };

    dm_input_element_desc tex_coords_element = {
        .name="TEX_COORDS",
        .class=DM_INPUT_ELEMENT_CLASS_PER_VERTEX,
        .format=DM_INPUT_ELEMENT_FORMAT_FLOAT_2,
        .stride=sizeof(nuklear_vertex),
        .offset=offsetof(nuklear_vertex, tex_coords),
    };

    dm_input_element_desc color_element = {
        .name="COLOR",
        .class=DM_INPUT_ELEMENT_CLASS_PER_VERTEX,
        .format=DM_INPUT_ELEMENT_FORMAT_FLOAT_4,
        .stride=sizeof(nuklear_vertex),
        .offset=offsetof(nuklear_vertex, color),
    };
    
    dm_raster_input_assembler_desc input_assembler = {
        .input_elements={ position_element,tex_coords_element,color_element }, .input_element_count=3,
        .topology=DM_INPUT_TOPOLOGY_TRIANGLE_LIST
    };

    dm_shader_desc vertex_shader_desc = {
#ifdef DM_DIRECTX12
        .path="assets/shaders/gui_vertex.cso"
#elif defined(DM_METAL)
        .path="assets/shaders/gui_vertex.metallib"
#elif defined(DM_VULKAN)
        .path="assets/shaders/gui_vertex.spv"
#endif
    };

    dm_shader_desc pixel_shader_desc = {
#ifdef DM_DIRECTX12
        .path="assets/shaders/gui_pixel.cso"
#elif defined(DM_METAL)
        .path="assets/shaders/gui_pixel.metallib"
#elif defined(DM_VULKAN)
        .path="assets/shaders/gui_pixel.spv"
#endif
    };

    dm_rasterizer_desc rasterizer_desc = {
        .cull_mode=DM_RASTERIZER_CULL_MODE_NONE, .front_face=DM_RASTERIZER_FRONT_FACE_COUNTER_CLOCKWISE, .polygon_fill=DM_RASTERIZER_POLYGON_FILL_FILL,
        .vertex_shader_desc=vertex_shader_desc, .pixel_shader_desc=pixel_shader_desc
    };

    dm_depth_stencil_desc depth_stencil = {
        .depth=false,
        .stencil=false
    };

    dm_raster_pipeline_desc pipeline_desc = {
        pipeline_desc.input_assembler=input_assembler,.rasterizer=rasterizer_desc,
    };

    if(!dm_create_raster_pipeline(pipeline_desc, &gui->pipeline, context)) { dm_free((void*)gui); return NULL; }

    // === nuklear initialization ===
    nk_buffer_init_default(&gui->cmds);

    nk_init_default(&gui->ctx, 0);

    gui->ctx.clip.copy     = nuklear_clipboard_copy;
    gui->ctx.clip.paste    = nuklear_clipboad_paste;
    gui->ctx.clip.userdata = nk_handle_ptr(0);

    // style
    struct nk_color table[NK_COLOR_COUNT];
    table[NK_COLOR_TEXT] = nk_rgba(210, 210, 210, 255);
    table[NK_COLOR_WINDOW] = nk_rgba(57, 67, 71, 215);
    table[NK_COLOR_HEADER] = nk_rgba(51, 51, 56, 220);
    table[NK_COLOR_BORDER] = nk_rgba(46, 46, 46, 255);
    table[NK_COLOR_BUTTON] = nk_rgba(48, 83, 111, 255);
    table[NK_COLOR_BUTTON_HOVER] = nk_rgba(58, 93, 121, 255);
    table[NK_COLOR_BUTTON_ACTIVE] = nk_rgba(63, 98, 126, 255);
    table[NK_COLOR_TOGGLE] = nk_rgba(50, 58, 61, 255);
    table[NK_COLOR_TOGGLE_HOVER] = nk_rgba(45, 53, 56, 255);
    table[NK_COLOR_TOGGLE_CURSOR] = nk_rgba(48, 83, 111, 255);
    table[NK_COLOR_SELECT] = nk_rgba(57, 67, 61, 255);
    table[NK_COLOR_SELECT_ACTIVE] = nk_rgba(48, 83, 111, 255);
    table[NK_COLOR_SLIDER] = nk_rgba(50, 58, 61, 255);
    table[NK_COLOR_SLIDER_CURSOR] = nk_rgba(48, 83, 111, 245);
    table[NK_COLOR_SLIDER_CURSOR_HOVER] = nk_rgba(53, 88, 116, 255);
    table[NK_COLOR_SLIDER_CURSOR_ACTIVE] = nk_rgba(58, 93, 121, 255);
    table[NK_COLOR_PROPERTY] = nk_rgba(50, 58, 61, 255);
    table[NK_COLOR_EDIT] = nk_rgba(50, 58, 61, 225);
    table[NK_COLOR_EDIT_CURSOR] = nk_rgba(210, 210, 210, 255);
    table[NK_COLOR_COMBO] = nk_rgba(50, 58, 61, 255);
    table[NK_COLOR_CHART] = nk_rgba(50, 58, 61, 255);
    table[NK_COLOR_CHART_COLOR] = nk_rgba(48, 83, 111, 255);
    table[NK_COLOR_CHART_COLOR_HIGHLIGHT] = nk_rgba(255, 0, 0, 255);
    table[NK_COLOR_SCROLLBAR] = nk_rgba(50, 58, 61, 255);
    table[NK_COLOR_SCROLLBAR_CURSOR] = nk_rgba(48, 83, 111, 255);
    table[NK_COLOR_SCROLLBAR_CURSOR_HOVER] = nk_rgba(53, 88, 116, 255);
    table[NK_COLOR_SCROLLBAR_CURSOR_ACTIVE] = nk_rgba(58, 93, 121, 255);
    table[NK_COLOR_TAB_HEADER] = nk_rgba(48, 83, 111, 255);
    nk_style_from_table(&gui->ctx, table);
    
    // fonts
    nk_font_atlas_init_default(&gui->atlas);
    nk_font_atlas_begin(&gui->atlas);

    for(uint8_t i=0; i<font_count; i++)
    {
        gui->fonts[i] = nk_font_atlas_add_from_file(&gui->atlas, font_paths[i], font_sizes[i], 0);

        int w,h;
        const void* data = nk_font_atlas_bake(&gui->atlas, &w,&h, NK_FONT_ATLAS_RGBA32);

        dm_texture_desc texture_desc = { 
            .width=w, .height=h,
            .format=DM_TEXTURE_FORMAT_BYTE_4_UNORM,.n_channels=4,
            .sampler=gui->sampler,
            .data=data
        };

        if(!dm_create_texture(texture_desc, &gui->font_texture, context)) { dm_free((void*)gui); return NULL; }
    }

    nk_font_atlas_end(&gui->atlas, nk_handle_ptr(0), &gui->null_texture);

    nk_style_set_font(&gui->ctx, &gui->fonts[0]->handle);

    return gui;
}

void gui_shutdown(gui_context* gui, dm_context* context)
{
    dm_free((void*)gui);
}

void gui_update_input(gui_context* gui, dm_context* context)
{
    uint16_t x,y;
    dm_input_get_mouse_pos(&x,&y, context);

    nk_input_begin(&gui->ctx);

    nk_input_motion(&gui->ctx, x,y);

    if(dm_input_is_mouse_button_pressed(DM_MOUSEBUTTON_L, context)) nk_input_button(&gui->ctx, NK_BUTTON_LEFT, x,y, 1);
    else                                                            nk_input_button(&gui->ctx, NK_BUTTON_LEFT, x,y, 0);
    if(dm_input_is_mouse_button_pressed(DM_MOUSEBUTTON_R, context)) nk_input_button(&gui->ctx, NK_BUTTON_RIGHT, x,y, 1);
    else                                                            nk_input_button(&gui->ctx, NK_BUTTON_RIGHT, x,y, 0);

    if(dm_input_is_key_pressed(DM_KEY_LCTRL, context))
    {
        if(dm_input_is_key_pressed(DM_KEY_U, context))          nk_input_key(&gui->ctx, NK_KEY_TEXT_UNDO, 1);
        else if(dm_input_is_key_pressed(DM_KEY_R, context))     nk_input_key(&gui->ctx, NK_KEY_TEXT_REDO, 1);
        else if(dm_input_is_key_pressed(DM_KEY_C, context))     nk_input_key(&gui->ctx, NK_KEY_COPY, 1);
        else if(dm_input_is_key_pressed(DM_KEY_V, context))     nk_input_key(&gui->ctx, NK_KEY_CUT, 1);
        else if(dm_input_is_key_pressed(DM_KEY_P, context))     nk_input_key(&gui->ctx, NK_KEY_PASTE, 1);
        else if(dm_input_is_key_pressed(DM_KEY_LEFT, context))  nk_input_key(&gui->ctx, NK_KEY_TEXT_WORD_LEFT, 1);
        else if(dm_input_is_key_pressed(DM_KEY_RIGHT, context)) nk_input_key(&gui->ctx, NK_KEY_TEXT_WORD_RIGHT, 1);
    }
    else
    {
        nk_input_key(&gui->ctx, NK_KEY_TEXT_UNDO, 0);
        nk_input_key(&gui->ctx, NK_KEY_TEXT_REDO, 0);
        nk_input_key(&gui->ctx, NK_KEY_COPY, 0);
        nk_input_key(&gui->ctx, NK_KEY_CUT, 0);
        nk_input_key(&gui->ctx, NK_KEY_PASTE, 0);
        nk_input_key(&gui->ctx, NK_KEY_TEXT_WORD_LEFT, 0);
        nk_input_key(&gui->ctx, NK_KEY_TEXT_WORD_RIGHT, 0);
    }

    nk_input_end(&gui->ctx);
}

void gui_update_buffers(gui_context* gui, dm_context* context)
{
    dm_render_command_update_constant_buffer(gui->cb, &gui->ortho, sizeof(gui->ortho), 0, context);

    // vertex and index buffers
    struct nk_convert_config config = { 0 };
    NK_STORAGE const struct nk_draw_vertex_layout_element vertex_layout[] = {
        { NK_VERTEX_POSITION, NK_FORMAT_FLOAT,              NK_OFFSETOF(nuklear_vertex, position) },
        { NK_VERTEX_TEXCOORD, NK_FORMAT_FLOAT,              NK_OFFSETOF(nuklear_vertex, tex_coords) },
        { NK_VERTEX_COLOR,    NK_FORMAT_R32G32B32A32_FLOAT, NK_OFFSETOF(nuklear_vertex, color) },
        { NK_VERTEX_LAYOUT_END }
    };

    config.vertex_layout = vertex_layout;
    config.vertex_size = sizeof(nuklear_vertex);
    config.vertex_alignment = NK_ALIGNOF(nuklear_vertex);
    config.global_alpha = 1.0f;
    config.shape_AA = NK_ANTI_ALIASING_ON;
    config.line_AA = NK_ANTI_ALIASING_ON;
    config.circle_segment_count = 22;
    config.curve_segment_count = 22;
    config.arc_segment_count = 22;
    config.tex_null = gui->null_texture;

    struct nk_buffer vbuf, ibuf;
    nk_buffer_init_fixed(&vbuf, gui->vertices, (size_t)MAX_NUKLEAR_VERTEX_BUFFER);
    nk_buffer_init_fixed(&ibuf, gui->indices, (size_t)MAX_NUKLEAR_INDEX_BUFFER);
    nk_convert(&gui->ctx, &gui->cmds, &vbuf, &ibuf, &config);

    dm_render_command_update_vertex_buffer(gui->vb, gui->vertices, sizeof(gui->vertices), 0, context);
    dm_render_command_update_index_buffer(gui->ib, gui->indices, sizeof(gui->indices), 0, context);
}

void gui_render(gui_context* gui, dm_context* context)
{
    gui->handles[0] = gui->cb;
    gui->handles[1] = gui->font_texture;
    gui->handles[2] = gui->sampler;

    dm_render_command_bind_raster_pipeline(gui->pipeline, context);
    dm_render_command_submit_resources(gui->handles, 3, context);
    dm_render_command_bind_vertex_buffer(gui->vb, 0, 0, context);
    dm_render_command_bind_index_buffer(gui->ib, 0, context);

    const struct nk_draw_command* cmd;
    uint32_t offset = 0;
    nk_draw_foreach(cmd, &gui->ctx, &gui->cmds)
    {
        if(!cmd->elem_count) continue;

        dm_render_command_draw_instanced_indexed(1,0,cmd->elem_count,offset,0, context);

        offset += cmd->elem_count;
    }
    
    nk_clear(&gui->ctx);
    nk_buffer_clear(&gui->cmds);
}
