
cbuffer CBPerObject : register(b0)
{
    float4x4 WorldViewProj;
};

struct VSInput
{
    float3 position : POSITION;
    float4 color    : COLOR;
};

struct VSOutput
{
    float4 position : SV_POSITION;
    float4 color    : COLOR;
};

VSOutput main(VSInput input)
{
    VSOutput output;
    output.position = mul(float4(input.position, 1.0f), WorldViewProj);
    output.color = input.color;
    return output;
}
