cbuffer CBLight : register(b1)
{
    float3 LightDirection;
    float pad1;
    float3 LightColor;
    float pad2;
    float3 CameraPosition;
    float pad3;
};

Texture2D DiffuseTexture : register(t0);
SamplerState TextureSampler : register(s0);

struct PSInput
{
    float4 position : SV_POSITION;
    float3 normalWS : NORMAL;
    float3 posWS : POSITION;
    float2 uv : TEXCOORD;
};

float4 main(PSInput input) : SV_TARGET
{
    float3 N = normalize(input.normalWS);
    float3 L = normalize(-LightDirection);
    float3 V = normalize(CameraPosition - input.posWS);
    float3 H = normalize(L + V);

    float diff = max(dot(N, L), 0.0f);
    float spec = pow(max(dot(N, H), 0.0f), 32.0f);
    
    float3 albedo = DiffuseTexture.Sample(TextureSampler, input.uv).rgb;

    float3 color = albedo * LightColor * (diff + spec * 0.25f);
    return float4(color, 1.0f);
}
