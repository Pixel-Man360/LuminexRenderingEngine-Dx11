cbuffer CBPerObject : register(b0)
{
    float4x4 World;
    float4x4 WorldInvTranspose;
    float4x4 View;
    float4x4 Projection;
    float4x4 LightViewProj;
    float4 SelectionColor; // xyz = color, w = is selected
};

struct VSInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

struct VSOutput
{
    float4 position : SV_POSITION;
    float3 normalWS : NORMAL;
    float3 posWS : POSITION;
    float2 uv : TEXCOORD;
    float4 posVS : TEXCOORD1;
    float4 selectionColor : TEXCOORD2;
};

VSOutput main(VSInput input)
{
    VSOutput output;

    float4 posWorld = mul(float4(input.position, 1.0f), World);
    output.posWS = posWorld.xyz;

    float4 posView = mul(posWorld, View);
    output.posVS = posView;

    output.position = mul(posView, Projection);

    output.normalWS = normalize(mul(input.normal, (float3x3) WorldInvTranspose));
    output.uv = input.uv;
    output.selectionColor = SelectionColor;

    return output;
}
