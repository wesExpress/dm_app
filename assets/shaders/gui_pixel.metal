#include <metal_stdlib>
using namespace metal;

struct fragment_in
{
    float4 position[[position]];
    float4 color;
    float2 uv;
    uint32_t texture;
};

struct camera_data
{
    float4x4 projection;
};

struct texture_argument_buffer
{
    array<texture2d<float>, 10> textures[[id(0)]];
    array<sampler, 10> samplers[[id(10)]];
};

struct resource_buffer 
{
    device camera_data* camera[[id(0)]];
    uint32_t font_texture;
    uint32_t sampler;
};

fragment float4 fragment_main(const device texture_argument_buffer& t_buffer[[buffer(0)]], const device resource_buffer& resources[[buffer(1)]], fragment_in frag[[stage_in]])
{
    return t_buffer.textures[2].sample(t_buffer.samplers[1], frag.uv) * frag.color;
}

