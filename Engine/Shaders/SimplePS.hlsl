#define LIGHT_DIRECTIONAL 0
#define LIGHT_POINT 1
#define MAX_LIGHTS 8
#define NUM_CASCADES 4
#define PI 3.14159265359

static const float SHADOW_MAP_SIZE = 2048.0f;

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

cbuffer CBShadow : register(b2)
{
    float4x4 LightViewProj[NUM_CASCADES];
    float4 CascadeSplits; // view-space split depths
};

cbuffer CBMaterial : register(b3)
{
    float4 MaterialAlbedo;
    float MaterialMetallic;
    float MaterialRoughness;
    float MaterialAO;
    float MaterialPadding1;
    
    float2 MaterialTiling;
    float2 MaterialOffset;
    
    uint UseAlbedoMap;
    uint UseNormalMap;
    uint UseMetallicMap;
    uint UseRoughnessMap;
    
    uint UseAOMap;
    uint3 MaterialPadding2;
};

Texture2D DiffuseTexture : register(t0);
Texture2DArray ShadowMapArray : register(t1);
Texture2D AlbedoMap : register(t2);
Texture2D NormalMap : register(t3);
Texture2D MetallicMap : register(t4);
Texture2D RoughnessMap : register(t5);
Texture2D AOMap : register(t6);
TextureCube IrradianceMap : register(t7);
TextureCube PrefilterMap : register(t8);
Texture2D BRDFLUT : register(t9);

SamplerState TextureSampler : register(s0);
SamplerComparisonState ShadowSampler : register(s1);
SamplerState IBLSampler : register(s2);

struct PSInput
{
    float4 position : SV_POSITION;
    float3 normalWS : NORMAL;
    float3 posWS : POSITION;
    float2 uv : TEXCOORD0;
    float4 posVS : TEXCOORD1;
    float4 selectionColor : TEXCOORD2;
};

// ----------------------------------------------------
// CASCADE SELECTION
// ----------------------------------------------------
int SelectCascade(float viewDepth)
{
    if (viewDepth < CascadeSplits.x)
        return 0;
    if (viewDepth < CascadeSplits.y)
        return 1;
    if (viewDepth < CascadeSplits.z)
        return 2;
    return 3;
}

// ----------------------------------------------------
// CASCADED SHADOW
// ----------------------------------------------------
float CalculateCascadedShadow(float3 posWS, float viewDepth,
                             float3 normalWS, float3 lightDir)
{
    int cascadeIndex = SelectCascade(viewDepth);

    float4 shadowPos = mul(float4(posWS, 1.0f), LightViewProj[cascadeIndex]);
    float3 proj = shadowPos.xyz / shadowPos.w;

    proj.xy = proj.xy * 0.5f + 0.5f;
    proj.y = 1.0f - proj.y;

    if (proj.x < 0 || proj.x > 1 ||
        proj.y < 0 || proj.y > 1 ||
        proj.z < 0 || proj.z > 1)
        return 1.0f;

    float bias = max(0.1f * (1.0f - dot(normalWS, lightDir)), 0.0002f);

    // Poisson disk samples for high-quality soft shadows
    static const float2 poissonDisk[16] = 
    {
        float2(-0.94201624, -0.39906216),
        float2(0.94558609, -0.76890725),
        float2(-0.094184101, -0.92938870),
        float2(0.34495938, 0.29387760),
        float2(-0.91588581, 0.45771432),
        float2(-0.81544232, -0.87912464),
        float2(-0.38277543, 0.27676845),
        float2(0.97484398, 0.75648379),
        float2(0.44323325, -0.97511554),
        float2(0.53742981, -0.47373420),
        float2(-0.26496911, -0.41893023),
        float2(0.79197514, 0.19090188),
        float2(-0.24188840, 0.99706507),
        float2(-0.81409955, 0.91437590),
        float2(0.19984126, 0.78641367),
        float2(0.14383161, -0.14100790)
    };
    
    // Softness radius - VERY large because shadow frustum covers huge area
    float softness = 7.0f / SHADOW_MAP_SIZE;
    
    float shadow = 0.0f;
    
    [unroll]
    for (int i = 0; i < 16; ++i)
    {
        float2 offset = poissonDisk[i] * softness;
        shadow += ShadowMapArray.SampleCmpLevelZero
        (
            ShadowSampler,
            float3(proj.xy + offset, cascadeIndex),
            proj.z - bias
        );
    }

    return shadow / 16.0f;
}

// ----------------------------------------------------
// PBR FUNCTIONS
// ----------------------------------------------------

float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    
    return a2 / max(denom, 0.0001);
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = GeometrySchlickGGX(NdotV, roughness);
    float ggx2 = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

float3 FresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(saturate(1.0 - cosTheta), 5.0);
}

float3 FresnelSchlickRoughness( float cosTheta,float3 F0, float roughness)
{
    return F0 + (max(float3(1.0 - roughness, 1.0 - roughness,  1.0 - roughness), F0) - F0) * pow(saturate(1.0 - cosTheta), 5.0);
}


float4 main(PSInput input) : SV_TARGET
{
    float3 N = normalize(input.normalWS);
    float3 V = normalize(CameraPosition - input.posWS); 
    float2 uv = input.uv * MaterialTiling + MaterialOffset;
    float3 albedo = MaterialAlbedo.rgb;
    
    if (UseAlbedoMap)
    {
        float3 texColor = AlbedoMap.Sample(TextureSampler, uv).rgb;
        albedo = texColor * MaterialAlbedo.rgb;
    }

    float metallic = UseMetallicMap ? MetallicMap.Sample(TextureSampler, uv).r : MaterialMetallic;
    float roughness = UseRoughnessMap ? RoughnessMap.Sample(TextureSampler, uv).r : MaterialRoughness;
    float ao = UseAOMap ? AOMap.Sample(TextureSampler, uv).r : MaterialAO;

  
    metallic = saturate(metallic);
    roughness = clamp(roughness, 0.04, 1.0);
    ao = saturate(ao);

    float viewDepth = abs(input.posVS.z);


    float3 F0 = lerp(float3(0.04, 0.04, 0.04), albedo, metallic);

    float3 Lo = float3(0.0, 0.0, 0.0);

    for (int i = 0; i < LightCount; ++i)
    {
        Light light = Lights[i];

        float3 L;
        float attenuation = 1.0f;
        float shadow = 1.0f;
        float3 radiance;

        if (light.Type == LIGHT_DIRECTIONAL)
        {
            L = normalize(-light.Direction);
            shadow = CalculateCascadedShadow(input.posWS, viewDepth, N, L);
            radiance = light.Color * light.Intensity;
        }
        else
        {
            float3 toLight = light.Position - input.posWS;
            float distance = length(toLight);
            L = toLight / distance;

            float rangeAttenuation = saturate(1.0f - (distance / light.Range));
            attenuation = rangeAttenuation * rangeAttenuation;
            
            radiance = light.Color * light.Intensity * attenuation;
        }

        float3 H = normalize(V + L);
        
        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        float3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);
        
        float3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        float3 specular = numerator / denominator;
        
        float3 kS = F;
        float3 kD = (1.0 - kS) * (1.0 - metallic);
        
        float NdotL = max(dot(N, L), 0.0);
        
        Lo += (kD * albedo / PI + specular) * radiance * NdotL * shadow;
    }

    float3 F = FresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    float3 kS = F;
    float3 kD = (1.0 - kS) * (1.0 - metallic);
    
    float3 irradiance = IrradianceMap.Sample(IBLSampler, N).rgb;
    float3 diffuseIBL = irradiance * albedo;
    
    float3 R = reflect(-V, N);
    const float MAX_REFLECTION_LOD = 4.0;
    float3 prefilteredColor = PrefilterMap.SampleLevel(IBLSampler, R, roughness * MAX_REFLECTION_LOD).rgb;
    float2 envBRDF = BRDFLUT.Sample(IBLSampler, float2(max(dot(N, V), 0.0), roughness)).rg;
    float3 specularIBL = prefilteredColor * (F * envBRDF.x + envBRDF.y);
    
    float3 ambient = (kD * diffuseIBL + specularIBL) * ao;
    float3 color = ambient + Lo;

   
    float3 x = color;
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    color = saturate((x * (a * x + b)) / (x * (c * x + d) + e));
    

    color = pow(color, 1.0 / 2.2);

    if (input.selectionColor.w > 0.5f)
    {
        color = lerp(color, input.selectionColor.xyz, 0.3f);
    }

    return float4(color, 1.0f);
}
