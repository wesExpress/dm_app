#ifndef __GUI_H__
#define __GUI_H__

#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_DEFAULT_FONT
#define NK_INCLUDE_STANDARD_IO
#include "Nuklear/nuklear.h"

#include "DarkMatter/dm.h"

#define MAX_NUKLEAR_VERTEX_BUFFER 5000 
#define MAX_NUKLEAR_INDEX_BUFFER  5000 

typedef struct nuklear_vertex_t
{
    vec2 position;
    vec2 tex_coords;
    vec4 color;
} nuklear_vertex;

typedef struct nuklear_resource_indices_t
{
    uint32_t camera_data_index;
    uint32_t font_texture_index;
    uint32_t sampler_index;
} nuklear_resource_indices;

#define MAX_NUKLEAR_FONTS 5
typedef struct gui_context_t 
{
    struct nk_context    ctx;
    struct nk_font_atlas atlas;
    struct nk_buffer     cmds;

    struct nk_font* fonts[MAX_NUKLEAR_FONTS];

    //
    dm_pipeline_handle pipeline;
    dm_resource_handle vb, ib;
    dm_resource_handle cb;
    dm_resource_handle sampler, font_texture;

    dm_resource_handle handles[10];

    nuklear_vertex vertices[MAX_NUKLEAR_VERTEX_BUFFER];
    uint16_t indices[MAX_NUKLEAR_INDEX_BUFFER];

    struct nk_draw_null_texture null_texture;

    uint32_t screen_width, screen_height;
    mat4 ortho;
} gui_context;

gui_context* gui_init(const char** font_paths, uint8_t* font_sizes, uint8_t font_count, dm_context* context);
void gui_shutdown(gui_context* gui, dm_context* context);
void gui_update_input(gui_context* gui, dm_context* context);
void gui_update_buffers(gui_context* gui, dm_context* context);
void gui_render(gui_context* gui, dm_context* context);

#endif // __GUI_H__
