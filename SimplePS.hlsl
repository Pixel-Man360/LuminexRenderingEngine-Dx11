cbuffer CBLight : register(b1)
{
    float3 LightDirection;
    float pad1;
    float3 LightColor;
    float pad2;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float3 normalWS : NORMAL;
    float3 posWS : POSITION;
};

float4 main(PSInput input) : SV_TARGET
{
    float3 N = normalize(input.normalWS);
    float3 L = normalize(-LightDirection);
    float3 V = normalize(-input.posWS);

    float3 H = normalize(L + V);

    float diff = max(dot(N, L), 0.0f);
    float spec = pow(max(dot(N, H), 0.0f), 200.0f);

    float3 color = LightColor * (diff + spec);
    return float4(color, 1.0f);
}
