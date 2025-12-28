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
Texture2D ShadowMap : register(t1);

SamplerState TextureSampler : register(s0);
SamplerComparisonState ShadowSampler : register(s1);

struct PSInput
{
    float4 position : SV_POSITION;
    float3 normalWS : NORMAL;
    float3 posWS : POSITION;
    float2 uv : TEXCOORD;
    float4 shadowPos : TEXCOORD1;
};

// ----------------------------------------------------
// SHADOW CALCULATION (Directional Light Only)
// ----------------------------------------------------
float CalculateShadow(float4 shadowPos)
{
    float3 projCoords = shadowPos.xyz / shadowPos.w;
    projCoords.xy = projCoords.xy * 0.5f + 0.5f;
    projCoords.y = 1.0f - projCoords.y;
    
    if (projCoords.x < 0.0f || projCoords.x > 1.0f ||
        projCoords.y < 0.0f || projCoords.y > 1.0f ||
        projCoords.z > 1.0f || projCoords.z < 0.0f)
    {
        return 1.0f;
    }
    
    // PCF (2x2 kernel)
    float shadow = 0.0f;
    float2 texelSize = 1.0f / float2(2048.0f, 2048.0f);
    
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            float2 offset = float2(x, y) * texelSize;
            shadow += ShadowMap.SampleCmpLevelZero(
                ShadowSampler,
                projCoords.xy + offset,
                projCoords.z
            );
        }
    }
    
    return shadow / 9.0f; // Average of 9 samples
}

float4 main(PSInput input) : SV_TARGET
{
    float3 N = normalize(input.normalWS);
    float3 V = normalize(CameraPosition - input.posWS);
    float3 albedo = DiffuseTexture.Sample(TextureSampler, input.uv).rgb;

    float3 color = 0.0f;

    for (int i = 0; i < LightCount; i++)
    {
        Light light = Lights[i];

        float3 L;
        float attenuation = 1.0f;
        float lightShadow = 1.0f;

        if (light.Type == LIGHT_DIRECTIONAL)
        {
            L = normalize(-light.Direction);
            lightShadow = CalculateShadow(input.shadowPos); // ✔ shadows apply only to directional light
        }
        else
        {
            float3 lightVec = light.Position - input.posWS;
            float dist = length(lightVec);
            L = lightVec / dist;

            attenuation = saturate(1.0f - dist / light.Range);
            attenuation *= attenuation;

            lightShadow = 1.0f; // ✔ point lights not shadowed (yet)
        }

        float diff = max(dot(N, L), 0.0f);

        float3 H = normalize(L + V);
        float spec = pow(max(dot(N, H), 0.0f), 16.0f);

        float3 lighting =
            (diff + spec * 0.25f) *
            light.Color *
            light.Intensity *
            attenuation *
            lightShadow;

        color += lighting;
    }

    return float4(albedo * color, 1.0f);
}
