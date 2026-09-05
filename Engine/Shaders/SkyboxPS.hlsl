cbuffer CBSkybox : register(b0)
{
    float4x4 InvProjection;
    float4x4 InvView;

    float ExposureEV;
    float SkyboxIntensity;
    float2 Padding;
};

TextureCube EnvironmentMap : register(t0);
SamplerState SkyboxSampler : register(s0);

struct PSInput
{
    float4 position : SV_POSITION;
    float3 directionWS : TEXCOORD0;
};

float3 ACESFilm(float3 color)
{
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;

    return saturate((color * (a * color + b)) / (color * (c * color + d) + e));
}

float4 main(PSInput input) : SV_TARGET
{
    float3 direction = normalize(input.directionWS);

    float3 color = EnvironmentMap.SampleLevel(SkyboxSampler, direction, 0.0f).rgb;
    color *= SkyboxIntensity;
    color *= exp2(ExposureEV);

    color = ACESFilm(color);
    color = pow(color, 1.0f / 2.2f);

    return float4(color, 1.0f);
}