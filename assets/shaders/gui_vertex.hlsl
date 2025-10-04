struct vertex 
{
    float2 position : POSITION;
    float2 uv       : TEX_COORDS0;
    float4 color    : COLOR0;
};

struct fragment
{
    float4 position : SV_Position;
    float4 color    : COLOR1;
    float2 uv       : TEX_COORDS1;
};

struct camera_data
{
    matrix projection;
};

struct resource_indices
{
    uint camera_buffer_index;
    uint font_texture_index;
    uint sampler_index;
};

ConstantBuffer<resource_indices> resources : register(b0);

fragment main(vertex v)
{
    ConstantBuffer<camera_data> camera_buffer = ResourceDescriptorHeap[resources.camera_buffer_index];

    fragment frag;

    frag.position = mul(float4(v.position.xy, 0, 1.f), camera_buffer.projection);
    frag.color    = v.color;
    frag.uv       = v.uv;

    return frag;
}
