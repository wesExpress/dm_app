#include <metal_stdlib>
using namespace metal;

struct fragment_in
{
    float4 position[[position]];
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

struct texture_argument_buffer
{
    array<texture2d<float>, 10> textures[[id(0)]];
    array<sampler, 10> samplers[[id(10)]];
};

struct resource_buffer 
{
    device camera_data* camera[[id(0)]];
    device instance* instances[[id(1)]];
};

fragment float4 fragment_main(const device texture_argument_buffer& t_buffer[[buffer(0)]], const device resource_buffer& resources[[buffer(1)]], fragment_in frag[[stage_in]])
{
    instance inst = resources.instances[frag.inst_id];

    texture2d<float> t1 = t_buffer.textures[inst.indices[0]];
    texture2d<float> t2 = t_buffer.textures[inst.indices[1]];
    sampler s = t_buffer.samplers[inst.indices[2]];

    float4 color1 = t1.sample(s, frag.uv);
    float4 color2 = t2.sample(s, frag.uv);

    return mix(color1, color2, 0.2f) * frag.color;
}

