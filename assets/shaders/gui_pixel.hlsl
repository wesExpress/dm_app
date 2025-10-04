struct fragment 
{
    float4 position : SV_Position;
    float4 color    : COLOR1;
    float2 uv       : TEX_COORDS1;
};

struct resource_indices
{
    uint camera_buffer_index;
    uint font_texture_index;
    uint sampler_index;
};

ConstantBuffer<resource_indices> resources : register(b0);

float4 main(fragment frag) : SV_Target
{
    Texture2D t = ResourceDescriptorHeap[resources.font_texture_index];
    SamplerState s = SamplerDescriptorHeap[resources.sampler_index];

    return t.Sample(s, frag.uv) * frag.color;
}
