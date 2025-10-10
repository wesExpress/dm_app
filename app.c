#include "DarkMatter/dm.h"
#include "gui.h"

typedef enum exit_code_t
{
    EXIT_CODE_SUCCESS = 0,
    EXIT_CODE_WINDOW_CLOSE = 0,
    EXIT_CODE_INIT_FAIL = -1,
    EXIT_CODE_RESOURCE_CREATE_FAIL = -2,
    EXIT_CODE_RESIZE_FAIL = -3,
    EXIT_CODE_RENDER_FAIL = -4,
    EXIT_CODE_UPDATE_FAIL = -5
} exit_code;

typedef struct vertex_t
{
    float position[4];
    float color[4];
    float uv[4];
} vertex;

typedef struct instance_t
{
    mat4 model;
    uint32_t indices[4];
} instance;

typedef struct simple_camera_t
{
    mat4 view, perspective, vp;
    vec3 position;
} simple_camera;

#define ENTITY_COUNT 10
typedef struct application_t
{
    dm_renderpass_handle pass;
    dm_pipeline_handle pipeline;
    dm_viewport_index viewport;
    dm_scissor_index scissor;
    dm_resource_handle vb, ib, cb;
    dm_resource_handle instance_buffer;
    dm_resource_handle texture, texture2;
    dm_resource_handle sampler;

    simple_camera camera;
    instance instances[ENTITY_COUNT];

    dm_context* context;
    gui_context* gui; 
} application;

vertex vertices[] = {
    { { -0.5f,-0.5f,0.f }, {1,0,0,1}, { 0,0 } },
    { {  0.5f,-0.5f,0.f }, {0,1,0,1}, { 1,0 } },
    { {  0.5f, 0.5f,0.f }, {0,0,1,1}, { 1,1 } },
    { { -0.5f, 0.5f,0.f }, {1,0,1,1}, { 0,1 } },
};

uint32_t indices[] = {
    0,1,2,
    2,3,0
};

bool app_init(application* app)
{
#ifdef DM_PLATFORM_APPLE
    uint32_t width = 1280, height = 720;
#else
    uint32_t width = 1920, height = 1080;
#endif
    app->context = dm_init(0,0,width,height, "test", DM_WINDOW_CREATE_FLAG_CENTER);
    if(!app->context) return false;

    app->context->flags |= DM_CONTEXT_FLAG_VSYNC_ON;

    return true;
}

void app_shutdown(application* app)
{
    gui_shutdown(app->gui, app->context);
    dm_shutdown(app->context);
}

bool create_resources(application* app)
{
    dm_renderpass_desc pass_desc = { .type=DM_RENDERPASS_TYPE_DEFAULT };
    if(!dm_create_renderpass(pass_desc, &app->pass, app->context)) return false;

    dm_input_element_desc position_element = {
        .name="POSITION",
        .class=DM_INPUT_ELEMENT_CLASS_PER_VERTEX,
        .format=DM_INPUT_ELEMENT_FORMAT_FLOAT_4,
        .stride=sizeof(vertex),
        .offset=offsetof(vertex, position),
    };

    dm_input_element_desc tex_coords_element = {
        .name="TEX_COORDS",
        .class=DM_INPUT_ELEMENT_CLASS_PER_VERTEX,
        .format=DM_INPUT_ELEMENT_FORMAT_FLOAT_2,
        .stride=sizeof(vertex),
        .offset=offsetof(vertex, uv),
    };

    dm_input_element_desc color_element = {
        .name="COLOR",
        .class=DM_INPUT_ELEMENT_CLASS_PER_VERTEX,
        .format=DM_INPUT_ELEMENT_FORMAT_FLOAT_4,
        .stride=sizeof(vertex),
        .offset=offsetof(vertex, color),
    };

    dm_rasterizer_desc rasterizer = {
#ifdef DM_DIRECTX12
        .vertex_shader_desc.path="assets/shaders/vertex.cso",
        .pixel_shader_desc.path="assets/shaders/pixel.cso",
#elif defined(DM_METAL)
        .vertex_shader_desc.path="assets/shaders/vertex_shader.metallib",
        .pixel_shader_desc.path="assets/shaders/pixel_shader.metallib",
#endif
        .cull_mode=DM_RASTERIZER_CULL_MODE_NONE, .front_face=DM_RASTERIZER_FRONT_FACE_COUNTER_CLOCKWISE,
        .polygon_fill=DM_RASTERIZER_POLYGON_FILL_FILL
    };

    dm_raster_input_assembler_desc input_assembler = {
        .input_elements={ position_element, color_element, tex_coords_element }, .input_element_count=3,
        .topology=DM_INPUT_TOPOLOGY_TRIANGLE_LIST
    };

    dm_raster_pipeline_desc pipe_desc = {
        .rasterizer=rasterizer, .input_assembler=input_assembler,
        .depth_stencil={ .depth=true }
    };

    if(!dm_create_raster_pipeline(pipe_desc, &app->pipeline, app->context)) return false;

    dm_viewport viewport = {
        .right=dm_get_window_width(app->context),
        .bottom=dm_get_window_height(app->context),
        .left=0,.top=0
    };

    dm_create_viewport(viewport, &app->viewport, app->context);

    dm_scissor scissor = {
        .right=dm_get_window_width(app->context),
        .bottom=dm_get_window_height(app->context),
        .left=0, .top=0
    };

    dm_create_scissor(scissor, &app->scissor, app->context);

    dm_vertex_buffer_desc vb_desc = {
        .size=sizeof(vertices), .stride=sizeof(vertex),
        .data=vertices
    };

    dm_index_buffer_desc ib_desc = {
        .size=sizeof(indices), .index_type=DM_INDEX_BUFFER_INDEX_TYPE_UINT32,
        .data=indices
    };

    if(!dm_create_vertex_buffer(vb_desc, &app->vb, app->context)) return false;
    if(!dm_create_index_buffer(ib_desc, &app->ib, app->context)) return false;

    // camera
    vec3 forward = { 0,0,1 };
    vec3 up = { 0,1,0 };
    vec3 target;
    glm_vec3_add(app->camera.position, forward, target);

    float fov = DM_DEG_TO_RAD(85.f); 
    float aspect = (float)dm_get_window_width(app->context) / (float)dm_get_window_height(app->context);

    glm_lookat(app->camera.position, target, up, app->camera.view);
    glm_perspective(fov, aspect, 0.1f, 100.f, app->camera.perspective);
    glm_mat4_mul(app->camera.perspective, app->camera.view, app->camera.vp);
#ifdef DM_DIRECTX12
    glm_mat4_transpose(app->camera.vp);
#endif

    dm_constant_buffer_desc cb_desc = {
        .size=sizeof(mat4),
        .data=app->camera.vp
    };

    if(!dm_create_constant_buffer(cb_desc, &app->cb, app->context)) return false;

    // instance buffer
    dm_storage_buffer_desc sb_desc = {
        .size=sizeof(app->instances), .stride=sizeof(instance),
        .data=app->instances
    };

    if(!dm_create_storage_buffer(sb_desc, &app->instance_buffer, app->context)) return false;

    // sampler
    dm_sampler_desc sampler_desc = {
        .address_u=DM_SAMPLER_ADDRESS_MODE_WRAP,
        .address_v=DM_SAMPLER_ADDRESS_MODE_WRAP,
        .address_w=DM_SAMPLER_ADDRESS_MODE_WRAP
    };

    if(!dm_create_sampler(sampler_desc, &app->sampler, app->context)) return false;

    // textures
    if(!dm_create_texture_from_file("assets/textures/container.jpg", &app->texture, app->context)) return false;
    if(!dm_create_texture_from_file("assets/textures/awesomeFace.png", &app->texture2, app->context)) return false;

    // gui 
    const char* font_paths[] = {
        "assets/fonts/JetBrainsMonoNerdFont-Regular.ttf"
    };

    uint8_t sizes[] = {
        16
    };

    app->gui = gui_init(font_paths, sizes, 1, app->context);
    if(!app->gui) return false;

    //
    if(!dm_finish_init(app->context)) return false;

    return true;
}

exit_code app_run(application* app)
{
    dm_timer timer = { 0 };
    double frame_time = 0;

    dm_timer_start(&timer);

    while(true)
    {
        dm_timer frame_timer = { 0 };
        dm_timer_start(&frame_timer);

        if(!dm_update(app->context)) return EXIT_CODE_UPDATE_FAIL;
        gui_update_input(app->gui, app->context);

        if(dm_input_is_key_pressed(DM_KEY_ESCAPE, app->context)) { dm_log(DM_LOG_WARN, "window closed"); return EXIT_CODE_WINDOW_CLOSE; }
        
        glm_perspective(DM_DEG_TO_RAD(85.f), (float)dm_get_window_width(app->context) / (float)dm_get_window_height(app->context), 0.1f, 100.f, app->camera.perspective);

        float move = 5.f * frame_time * 0.001f;

        if(dm_input_is_key_pressed(DM_KEY_LEFT, app->context))       app->camera.position[0] += move; 
        else if(dm_input_is_key_pressed(DM_KEY_RIGHT, app->context)) app->camera.position[0] -= move;

        if(dm_input_is_key_pressed(DM_KEY_UP, app->context))        app->camera.position[2] += move;
        else if(dm_input_is_key_pressed(DM_KEY_DOWN, app->context)) app->camera.position[2] -= move;

        vec3 forward = { 0,0,1 };
        vec3 up = { 0,1,0 };

        vec3 target;
        glm_vec3_add(app->camera.position, forward, target);

        glm_lookat(app->camera.position, target, up, app->camera.view);
        glm_mul(app->camera.perspective, app->camera.view, app->camera.vp);
#ifdef DM_DIRECTX12
        glm_mat4_transpose(app->camera.vp);
#endif

        for(uint8_t i=0; i<ENTITY_COUNT; i++)
        {
            float x = (float)i / (float)ENTITY_COUNT * 10.f - 5.f;
            glm_mat4_identity(app->instances[i].model);
            glm_translate(app->instances[i].model, (vec3){ x,x,x });
#ifdef DM_DIRECTX12
            glm_mat4_transpose(app->instances[i].model);
#endif

            if(i % 2) app->instances[i].indices[0] = dm_get_resource_index(app->texture2, app->context);
            else      app->instances[i].indices[0] = dm_get_resource_index(app->texture, app->context);

            app->instances[i].indices[1] = dm_get_resource_index(app->sampler, app->context);
        }

        // gui test
        char buffer[512];

#if 1
        nk_style_set_font(&app->gui->ctx, &app->gui->fonts[0]->handle);
        if(nk_begin(&app->gui->ctx,"Test", nk_rect(100,100, 250,550), NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_TITLE | NK_WINDOW_MINIMIZABLE | NK_WINDOW_DYNAMIC | NK_WINDOW_SCALABLE))
        {
            nk_layout_row_dynamic(&app->gui->ctx, 100, 1);
            char buffer[512];
            sprintf(buffer, "Frame time: %lf ms", frame_time);
            nk_label(&app->gui->ctx, buffer, NK_TEXT_LEFT);

            nk_end(&app->gui->ctx);
        }
#else
        dm_log(DM_LOG_WARN, "%lf", frame_time);
#endif

        dm_resource_handle resources[] = { app->cb,app->instance_buffer };

        // rendering
        dm_render_command_begin_frame(app->context);

            dm_render_command_begin_update(app->context);
                dm_render_command_update_constant_buffer(app->cb, app->camera.vp, sizeof(mat4), 0, app->context);
                dm_render_command_update_storage_buffer(app->instance_buffer, app->instances, sizeof(app->instances), 0, app->context);

                //gui_update_buffers(app->gui, app->context);
            dm_render_command_end_update(app->context);

            dm_render_command_begin_render_pass(app->pass, 0.5f,0.7f,0.9f,1,1, app->context);
                dm_render_command_set_viewport(app->viewport, app->context);
                dm_render_command_set_scissor(app->scissor, app->context);

                dm_render_command_bind_raster_pipeline(app->pipeline, app->context);
                dm_render_command_submit_resources(resources, DM_COUNTOF(resources), app->context);
                dm_render_command_bind_vertex_buffer(app->vb, 0, 0, app->context);
                dm_render_command_bind_index_buffer(app->ib, 0, app->context);
                dm_render_command_draw_instanced_indexed(ENTITY_COUNT,0,6,0,0, app->context);

                //gui_render(app->gui, app->context);
            dm_render_command_end_render_pass(app->pass, app->context);

        dm_render_command_end_frame(app->context);

        if(!dm_submit_render_commands(app->context)) { dm_log(DM_LOG_FATAL, "submit commands failed"); return EXIT_CODE_RENDER_FAIL; }
        
        frame_time = dm_timer_elapsed_ms(&frame_timer);
    }

    return EXIT_CODE_SUCCESS;
}

/********
 * MAIN *
 ********/
int main()
{
    application app = { 0 };

    if(!app_init(&app)) return EXIT_CODE_INIT_FAIL;

    if(!create_resources(&app))
    {
        app_shutdown(&app);
        return EXIT_CODE_RESOURCE_CREATE_FAIL;
    }

    exit_code code = app_run(&app);
    app_shutdown(&app);

    return code;
}
