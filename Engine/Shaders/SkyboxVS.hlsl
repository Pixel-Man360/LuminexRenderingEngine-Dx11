cbuffer CBSkybox : register(b0)
{
    float4x4 InvProjection;
    float4x4 InvView;

    float ExposureEV;
    float SkyboxIntensity;
    float2 Padding;
};

struct VSInput
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
};

struct VSOutput
{
    float4 position : SV_POSITION;
    float3 directionWS : TEXCOORD0;
};

VSOutput main(VSInput input)
{
    VSOutput output;

    output.position = float4(input.position.xy, 0.0f, 1.0f);

    float4 viewPosition = mul(float4(input.position.xy, 1.0f, 1.0f), InvProjection);
    viewPosition.xyz /= viewPosition.w;

    float3 directionVS = normalize(viewPosition.xyz);
    output.directionWS = normalize(mul(float4(directionVS, 0.0f), InvView).xyz);

    return output;
}