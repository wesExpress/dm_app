#include <metal_stdlib>
using namespace metal;

struct vertex_in
{
    float4 position;
    float4 color;
    float4 uv;
};

struct fragment_out
{
    float4 position [[position]];
    float4 color;
    float2 uv;
    uint32_t inst_id;
};

struct camera_data
{
    float4x4 projection;
};

struct instance
{
    float4x4 model;
    uint32_t indices[4];
};

struct resource_buffer 
{
    device camera_data* camera[[id(0)]];
    device instance* instances[[id(1)]];
};

vertex fragment_out vertex_main(const device resource_buffer& resources[[buffer(0)]], const device vertex_in* vertices[[buffer(1)]], uint v_id[[vertex_id]], uint inst_id[[instance_id]])
{
    fragment_out frag;

    instance inst = resources.instances[inst_id];

    float4x4 model      = inst.model;
    float4x4 projection = resources.camera->projection;

    frag.position = float4(vertices[v_id].position.xyz, 1.f);
    frag.position = model * frag.position;
    frag.position = projection * frag.position;
    frag.color    = vertices[v_id].color;
    frag.uv       = vertices[v_id].uv.xy;
    frag.inst_id  = inst_id;

    return frag;
}

