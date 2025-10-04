struct vertex
{
    float4 position : POSITION;
    float4 color    : COLOR0;
    float4 uv       : TEX_COORDS0;
};

struct fragment
{
    float4 position : SV_Position;
    float4 color    : COLOR1;
    float2 uv       : TEX_COORDS1;
    uint   inst_id  : INSTANCE;
};

struct camera_data
{
    float4x4 projection;
};

struct instance
{
    matrix model;
    uint   texture;
    uint   sampler;
};

struct resource_indices
{
    uint camera_buffer_index;
    uint instance_buffer_index;
};

ConstantBuffer<resource_indices> resources : register(b0);

fragment main(vertex v, uint inst_id : SV_InstanceID)
{
    fragment frag;

    ConstantBuffer<camera_data> camera_buffer  = ResourceDescriptorHeap[resources.camera_buffer_index]; 
    StructuredBuffer<instance> instance_buffer = ResourceDescriptorHeap[resources.instance_buffer_index];
    
    matrix model = instance_buffer[inst_id].model;

    frag.position = mul(float4(v.position.xyz, 1.f), model);
    frag.position = mul(frag.position, camera_buffer.projection);
    frag.color    = v.color;
    frag.uv       = v.uv.xy;
    frag.inst_id  = inst_id;

    return frag;
}

