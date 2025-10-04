struct fragment 
{
    float4 position : SV_Position;
    float4 color    : COLOR1;
    float2 uv       : TEX_COORDS1;
    uint   inst_id  : INSTANCE;
};

struct instance 
{
    matrix model;
    uint   texture_index;
    uint   sampler_index;
};

struct resource_indices
{
    uint camera_buffer_index;
    uint instance_buffer_index;
};

ConstantBuffer<resource_indices> resources : register(b0);

float4 main(fragment frag) : SV_Target
{
    StructuredBuffer<instance> instance_buffer = ResourceDescriptorHeap[resources.instance_buffer_index];
    
    instance inst = instance_buffer[frag.inst_id];

    Texture2D    t = ResourceDescriptorHeap[inst.texture_index];
    SamplerState s = SamplerDescriptorHeap[inst.sampler_index];

    return t.Sample(s, frag.uv);
}

