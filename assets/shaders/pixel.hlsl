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
    uint   indices[4];
};

struct resource_indices
{
    uint camera_buffer_index;
    uint instance_buffer_index;
};

ConstantBuffer<resource_indices> resources : register(b0);

float4 main(fragment frag) : SV_Target
{
    StructuredBuffer<instance> instance_buffer = ResourceDescriptorHeap[resources.instance_buffer_index+1];
    
    instance inst = instance_buffer[frag.inst_id];

    Texture2D    t = ResourceDescriptorHeap[inst.indices[0]+1];
    SamplerState s = SamplerDescriptorHeap[inst.indices[1]];

    return t.Sample(s, frag.uv);
}

