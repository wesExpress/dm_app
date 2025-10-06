#include <metal_stdlib>
using namespace metal;

struct vertex_in
{
    packed_float2 position;
    packed_float2 uv;
    float4 color;
};

struct fragment_out
{
    float4 position [[position]];
    float4 color;
    float2 uv;
};

struct camera_data
{
    float4x4 projection;
};

struct resource_buffer 
{
    device camera_data* camera[[id(0)]];
    texture2d<float> font_texture[[id(1)]];
    sampler s[[id(2)]];
};

vertex fragment_out vertex_main(const device resource_buffer& resources[[buffer(0)]], const device vertex_in* vertices[[buffer(1)]], uint v_id[[vertex_id]], uint inst_id[[instance_id]])
{
    fragment_out frag;

    frag.position = float4(vertices[v_id].position.xy, 0, 1.f);
    frag.position = resources.camera->projection * frag.position;
    frag.color    = vertices[v_id].color;
    frag.uv       = vertices[v_id].uv;

    return frag;
}
