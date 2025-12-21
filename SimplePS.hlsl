#define LIGHT_DIRECTIONAL 0
#define LIGHT_POINT 1
#define MAX_LIGHTS 8

struct Light
{
    int Type;
    float3 Color;

    float3 Direction;
    float Range;

    float3 Position;
    float Intensity;
};

cbuffer CBLight : register(b1)
{
    int LightCount;
    float3 CameraPosition;
    Light Lights[MAX_LIGHTS];
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
    float3 V = normalize(CameraPosition - input.posWS);
    float3 albedo = DiffuseTexture.Sample(TextureSampler, input.uv).rgb;

    float3 color = 0;

    for (int i = 0; i < LightCount; i++)
    {
        Light light = Lights[i];

        float3 L;
        float attenuation = 1.0f;

        if (light.Type == LIGHT_DIRECTIONAL)
        {
            L = normalize(-light.Direction);
        }
        else
        {
            float3 lightVec = light.Position - input.posWS;
            float dist = length(lightVec);
            L = lightVec / dist;

            attenuation = saturate(1.0f - dist / light.Range);
            attenuation *= attenuation;
        }

        float diff = max(dot(N, L), 0.0f);

        float3 H = normalize(L + V);
        float spec = pow(max(dot(N, H), 0.0f), 16.0f);

        float3 lighting =
            (diff + spec * 0.25f) *
            light.Color *
            light.Intensity *
            attenuation;

        color += lighting;
    }

    return float4(albedo * color, 1.0f);
}
