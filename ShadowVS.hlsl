cbuffer CBPerObject : register(b0)
{
    float4x4 World;
    float4x4 WorldInvTranspose;
    float4x4 View;
    float4x4 Projection;
    float4x4 LightViewProj;
};

struct VSInput
{
    float3 position : POSITION;
};

struct VSOutput
{
    float4 position : SV_POSITION;
};

VSOutput main(VSInput input)
{
    VSOutput output;
    
    float4 worldPosition = mul(float4(input.position, 1.0f), World);
    output.position = mul(worldPosition, LightViewProj);
    return output;
}